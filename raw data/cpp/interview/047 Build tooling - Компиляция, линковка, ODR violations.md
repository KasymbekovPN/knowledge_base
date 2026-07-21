[[raw data/cpp/interview/_|<=]]

# Build/tooling: компиляция, линковка, ODR violations

Мы разобрали каждую тему по отдельности (компиляция в header/cpp, линковка, ODR). Здесь — **практический tooling-срез**: как это выглядит с точки зрения инструментов сборки, флагов и диагностики. Это то, что спрашивают в контексте «покажи, что умеешь собирать и отлаживать проект».

## Полный конвейер с флагами

```bash
# 1. Препроцессинг — раскрытие #include, макросов
g++ -E main.cpp -o main.i          # посмотреть, во что превратился файл

# 2. Компиляция → ассемблер
g++ -S main.cpp -o main.s          # посмотреть сгенерированный ассемблер

# 3. Ассемблирование → объектный файл
g++ -c main.cpp -o main.o          # ОДНА translation unit → .o (НЕ линкуется)

# 4. Линковка → исполняемый
g++ main.o util.o -o program       # сшивка + библиотеки

# всё сразу:
g++ main.cpp util.cpp -o program
```

Ключевой флаг для понимания раздельной компиляции — **`-c`**: компилирует до `.o`, **без линковки**. Так собирают проект по файлам, потом линкуют разом.

---

## Диагностика: что смотреть в объектных файлах

### Таблица символов

```bash
nm main.o                          # символы объектного файла
# 0000000000000000 T _Z3fooi       T = defined в .text (наш foo(int))
#                  U _Z6helperv     U = undefined (нужен извне — helper())

nm -C main.o                       # -C = demangle (человекочитаемо)
# T foo(int)
# U helper()
```

`nm` — первый инструмент при ошибках линковки. `T` — символ определён, `U` — требуется извне. Ищешь, кто определяет `U`-символ.

```bash
nm -C --defined-only libmath.a     # что БИБЛИОТЕКА предоставляет
nm -C -u main.o                     # что TU ТРЕБУЕТ (undefined)
```

### Demangling вручную

```bash
c++filt _Z3fooi                    # _Z3fooi → foo(int)
echo "_ZN2ns6WidgetC1Ei" | c++filt # ns::Widget::Widget(int)
```

Полезно, когда ошибка линковки показывает mangled имя.

### Зависимости и содержимое

```bash
ldd program                        # динамические зависимости (.so)
objdump -d main.o                  # дизассемблировать
objdump -t main.o                  # таблица символов (альтернатива nm)
readelf -s program                 # символы ELF
readelf -d program                 # динамическая секция (нужные .so, SONAME, rpath)
ar t libmath.a                     # содержимое статической библиотеки (список .o)
```

---

## Диагностика ODR violations на практике

ODR-нарушения (мы разбирали) — самые коварные, потому что правило 3 (разные определения) линкер обычно **не ловит**. Инструменты:

### 1. Линкер-детектор (частичный)

```bash
g++ -fuse-ld=gold -Wl,--detect-odr-violations main.o util.o
# gold linker сравнивает определения с одинаковым именем в разных TU
# ловит ЧАСТЬ нарушений (когда доступна debug-инфо)
```

### 2. LTO выявляет несовпадения

```bash
g++ -flto -O2 a.cpp b.cpp -o program
# LTO видит ВЕСЬ код → замечает разные определения одного символа
# warning: type 'Widget' violates the C++ One Definition Rule
```

LTO — побочно один из лучших детекторов ODR, потому что сводит все TU вместе.

### 3. ASan — ODR для глобальных переменных

```bash
g++ -fsanitize=address a.cpp b.cpp -o program
./program
# ==ERROR: AddressSanitizer: odr-violation (0x...):
#   size of global 'globalVar' differ in two TUs
```

ASan ловит ODR для **глобальных переменных с разным размером** (частый симптом разных определений класса).

### 4. Единые флаги — предотвращение

Мы разбирали GCC dual ABI — это тоже вид ODR-нарушения (разный layout `std::string` при `_GLIBCXX_USE_CXX11_ABI=0/1`):

```bash
# ⚠️ смешивание → undefined reference to `foo(std::__cxx11::string)`
g++ -D_GLIBCXX_USE_CXX11_ABI=0 a.cpp   # старый ABI
g++ -D_GLIBCXX_USE_CXX11_ABI=1 b.cpp   # новый ABI
# → ODR violation через несовместимый layout string

# ✅ единые флаги для ВСЕХ TU
```

Правило: **все TU компилируются одинаковыми флагами**, влияющими на layout (`-std`, `-D`, ABI-флаги, `-m32/-m64`, структурные `#pragma pack`).

---

## Типичные ошибки и их диагностика

### `undefined reference` — разбор по шагам

```bash
# undefined reference to `helper()'
```

Алгоритм:

```bash
# 1. Кто ТРЕБУЕТ символ?
nm -C -u main.o | grep helper    # → U helper()  — main.o нужен helper

# 2. Кто ОПРЕДЕЛЯЕТ? (ищем по всем .o и библиотекам)
nm -C util.o | grep helper       # пусто? → helper нигде не определён
nm -C libX.a | grep helper       # проверяем библиотеки

# причины:
# - забыт .cpp/.o с определением при линковке
# - неверная сигнатура (mangling mismatch): void f(int) vs void f(long)
# - забыт -lLIBRARY
# - неверный порядок библиотек (библиотека ДО объектника)
# - забыт extern "C" (C++ ждёт _Z..., C даёт unmangled)
```

### Mangling mismatch — увидеть глазами

```bash
# main.cpp вызывает f(int) → ищет _Z1fi
# lib.cpp определил f(long) → дал _Z1fl
nm -C -u main.o | grep f    # U f(int)
nm -C lib.o | grep f        # T f(long)   ← НЕ совпадает!
```

### `multiple definition` — ODR правило 2

```bash
# multiple definition of `add(int, int)'
nm -C a.o | grep add    # T add(int, int)
nm -C b.o | grep add    # T add(int, int)   ← оба ОПРЕДЕЛЯЮТ
# причина: определение (не inline) в заголовке → в каждой TU
# решение: inline, или объявление в .h + определение в одном .cpp
```

### `undefined reference to vtable`

```bash
# undefined reference to `vtable for Widget'
# причина: первая non-inline виртуальная функция не определена
# (компилятор эмитит vtable в TU, где она определена)
# решение: определить хотя бы одну (обычно ~Widget()) в .cpp
```

---

## Флаги предупреждений — ловить проблемы рано

```bash
g++ -Wall -Wextra -Wpedantic       # базовый набор — ВСЕГДА
g++ -Werror                        # предупреждения → ошибки (CI)

# специфичные для наших тем:
-Wodr                              # ODR-нарушения (с LTO)
-Wsuggest-override                 # забытый override (разбирали vtable)
-Wnon-virtual-dtor                 # полиморфный класс без virtual деструктора
-Wshadow                           # затенение переменных
-Wconversion                       # неявные сужающие преобразования (signed/unsigned)
-Wsign-compare                     # signed/unsigned сравнения (разбирали UB)
-Wsequence-point                   # UB порядка вычисления (разбирали)
-Wreturn-local-addr                # dangling — возврат адреса локальной (разбирали)
-Wstrict-aliasing=2                # нарушения strict aliasing (разбирали)
```

Практика: `-Wall -Wextra -Werror` в CI обязательны. Многие UB, что мы разбирали, ловятся предупреждениями на этапе компиляции.

---

## Санитайзеры — рантайм-диагностика

Сводка того, что мы разбирали по UB:

```bash
# ASan — память: use-after-free, buffer overflow, ODR (globals), leaks
g++ -fsanitize=address -g prog.cpp
# замедление ~2x

# UBSan — undefined behavior: signed overflow, некорректные сдвиги,
#         разыменование null, misaligned, INT_MIN/-1
g++ -fsanitize=undefined -g prog.cpp
# замедление ~1.2x

# TSan — data races (разбирали в concurrency)
g++ -fsanitize=thread -g prog.cpp
# замедление ~5-15x

# комбо (ASan+UBSan совместимы):
g++ -fsanitize=address,undefined -g prog.cpp
```

**ASan и TSan несовместимы** (оба инструментируют память по-разному) — запускай раздельно. В CI обычно: сборка с ASan+UBSan для функциональных тестов + отдельная с TSan для многопоточных.

---

## Debug-режим стандартной библиотеки

```bash
# libstdc++ — проверка инвалидации итераторов, границ (разбирали)
g++ -D_GLIBCXX_DEBUG prog.cpp        # полный debug-режим контейнеров
g++ -D_GLIBCXX_ASSERTIONS prog.cpp   # лёгкий — проверка границ, предусловий

# libc++ (Clang):
clang++ -D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_DEBUG
```

`_GLIBCXX_DEBUG` ловит использование инвалидированных итераторов (мы разбирали инвалидацию) с внятным сообщением вместо тихого UB. **Важно:** меняет ABI контейнеров → ВСЕ TU должны собираться с ним (иначе — ODR violation!).

---

## Ускорение сборки — практика для больших проектов

Раз у тебя большой монолит — это критично:

```bash
# 1. Параллельная сборка
make -j$(nproc)                    # или ninja (быстрее make)

# 2. ccache — кэш компиляции
ccache g++ ...                     # переиспользует .o для неизменённых TU

# 3. Прекомпилированные заголовки (PCH)
g++ -x c++-header common.h -o common.h.gch   # тяжёлые стабильные заголовки

# 4. Unity build — объединить .cpp (меньше overhead на TU)
#    (осторожно — усугубляет ODR-проблемы с anonymous namespace)

# 5. LTO только для release (медленно линкует)
g++ -flto -O2 ...                  # release
g++ -O0 -g ...                     # debug — быстрая сборка
```

Главные рычаги сокращения времени пересборки (связано с header/cpp и pImpl, что мы разбирали):

- **Минимум `#include` в заголовках** (forward declaration)
- **pImpl** — разрыв зависимостей компиляции
- **ccache** — не пересобирать неизменённое
- **ninja** вместо make
- **Модули C++20** — стратегически

---

## CMake — оркестрация (твой стек)

Раз ты глубоко изучал CMake, свяжу с темами:

```cmake
# разделение на TU и линковка
add_library(mathlib STATIC add.cpp sub.cpp)      # статическая .a
add_library(mathlib SHARED add.cpp sub.cpp)      # динамическая .so (+ -fPIC авто)
add_executable(program main.cpp)
target_link_libraries(program PRIVATE mathlib)   # линковка

# единые флаги для ВСЕХ TU (предотвращение ODR/ABI mismatch)
set(CMAKE_CXX_STANDARD 20)
target_compile_options(program PRIVATE -Wall -Wextra)

# санитайзеры
target_compile_options(program PRIVATE -fsanitize=address,undefined)
target_link_options(program PRIVATE -fsanitize=address,undefined)

# LTO
set_target_properties(program PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON)

# PIC для shared
set_target_properties(mathlib PROPERTIES POSITION_INDEPENDENT_CODE ON)
```

CMake `PRIVATE/PUBLIC/INTERFACE` при линковке — прямое управление транзитивными зависимостями (тем, что мы разбирали в header/cpp):

- `PRIVATE` — зависимость только для реализации (не протекает в потребителей)
- `PUBLIC` — и для реализации, и для потребителей (протекает)
- `INTERFACE` — только для потребителей (header-only)

Правильное использование `PRIVATE` = меньше транзитивных зависимостей = быстрее сборка.

---

## Формулировки на собеседовании

**«Как диагностировать undefined reference?»** — `nm -C -u main.o` — кто **требует** символ; `nm -C` по всем .o/библиотекам — кто **определяет**. Причины: забыт .cpp/библиотека, mangling mismatch (неверная сигнатура), неверный порядок статических библиотек, забыт `extern "C"`.

**«Как поймать ODR violation?»** — Линкер обычно не ловит правило 3 (разные определения). Инструменты: `-fuse-ld=gold -Wl,--detect-odr-violations`, **LTO** (видит весь код), **ASan** (для глобальных переменных разного размера). Профилактика — единые флаги компиляции для всех TU.

**«Что такое `-c`?»** — Компиляция одной TU до объектного файла **без линковки**. Основа раздельной компиляции: собираем файлы по отдельности, линкуем разом.

**«Инструменты для символов?»** — `nm` (таблица символов, `-C` для demangle, `-u` для undefined), `c++filt` (demangle вручную), `ldd` (зависимости .so), `objdump`/`readelf` (детальный разбор), `ar t` (содержимое .a).

**«Какие санитайзеры и что ловят?»** — ASan (use-after-free, buffer overflow, ODR globals, leaks), UBSan (signed overflow, сдвиги, null-разыменование), TSan (data races). ASan+UBSan совместимы; TSan отдельно.

**«Как ускорить сборку большого проекта?»** — ccache (кэш .o), ninja вместо make, `-j` (параллельно), PCH, forward declaration + pImpl (разрыв зависимостей заголовков), LTO только для release. Главный рычаг — минимизация зависимостей компиляции через заголовки.

**«Почему `_GLIBCXX_DEBUG` требует единых флагов?»** — Он меняет layout контейнеров (добавляет проверки) → разный `sizeof` в TU с флагом и без → **ODR violation**. Все TU должны собираться одинаково.

---

Отличие от Java: там весь этот tooling-слой **радикально проще**. (1) Нет отдельных фаз препроцессор/компиляция/ассемблирование/линковка — `javac` сразу даёт `.class`, связывание динамическое в рантайме. (2) Нет `nm`/`c++filt`/mangling-диагностики — символы человекочитаемы, ошибки (`NoSuchMethodError`) в рантайме с ясным сообщением. (3) Нет ODR-инструментов — проблемы не существует. (4) Нет санитайзеров памяти — GC исключает use-after-free/leaks by design (есть профайлеры для утечек логических ссылок, но не memory corruption). (5) Сборка (Maven/Gradle) оркеструет зависимости и компиляцию, но нет флагов ABI/layout/санитайзеров — нечего настраивать на этом уровне. Ключевое: в C++ значительная часть инженерного времени уходит на **build/link/UB диагностику** (отсюда богатый tooling — nm, санитайзеры, LTO, ccache), в Java этот слой почти отсутствует, зато больше внимания рантайм-профилированию (JVM, GC-тюнинг, JIT). Для тебя это смена фокуса: Java-инженер думает про heap-профили и GC-паузы, C++-инженер — про символы, линковку, санитайзеры и время компиляции. Твой профиль (CMake, Docker, санитайзеры уже в изучении) показывает, что ты этот слой осваиваешь системно — на собеседовании умение продиагностировать undefined reference через `nm` или поймать UB санитайзером выделяет практика от теоретика.
