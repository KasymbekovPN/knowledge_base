[[raw data/cpp/interview/_|<=]]

Раз ты уже прошёл полный курс по CMake (от минимального `CMakeLists.txt` до CPack/WiX, пресетов, CTest и кросс-компиляции), базовую часть пробегу конспективно — как шпаргалку для собеседования — и сосредоточусь на санитайзерах и valgrind.

# CMake — минимум для собеседования

```cmake
cmake_minimum_required(VERSION 3.20)
project(MyApp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)          # -std=c++20, а не -std=gnu++20

add_library(mathlib STATIC add.cpp sub.cpp)
target_include_directories(mathlib PUBLIC include)   # PUBLIC → протекает потребителям

add_executable(app main.cpp)
target_link_libraries(app PRIVATE mathlib)
```

**Что реально спрашивают:**

**PRIVATE / PUBLIC / INTERFACE** — управление транзитивностью (прямо связано с header/cpp зависимостями, что мы разбирали):

- `PRIVATE` — только для реализации цели, **не** протекает потребителям
- `PUBLIC` — и для реализации, и для потребителей (заголовок цели включает заголовок зависимости)
- `INTERFACE` — только для потребителей (header-only библиотека)

Правильный `PRIVATE` = меньше транзитивных зависимостей = быстрее сборка. Это главный «архитектурный» вопрос по CMake.

**Modern CMake (target-based) vs старый стиль:**

```cmake
# ❌ старый — глобальные переменные, всё протекает всем
include_directories(include)
add_definitions(-DFOO)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# ✅ современный — свойства целей
target_include_directories(app PRIVATE include)
target_compile_definitions(app PRIVATE FOO)
target_compile_options(app PRIVATE -Wall)
```

**Типы сборки:**

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug        # -O0 -g
cmake -B build -DCMAKE_BUILD_TYPE=Release      # -O3 -DNDEBUG
cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo  # -O2 -g  ← для профилирования
cmake --build build -j
```

---

# Sanitizers

Инструментация **на этапе компиляции**: компилятор вставляет проверки в код. Отсюда — нужна перекомпиляция, но скорость и точность выше, чем у эмуляторов вроде valgrind.

## ASan — AddressSanitizer

```bash
g++ -fsanitize=address -fno-omit-frame-pointer -g -O1 prog.cpp
```

**Что ловит** (всё, что мы разбирали в UB):

- use-after-free
- heap/stack/global buffer overflow
- use-after-return (нужен `ASAN_OPTIONS=detect_stack_use_after_return=1`)
- use-after-scope (`-fsanitize-address-use-after-scope`)
- double-free, invalid free
- **memory leaks** (LeakSanitizer встроен, Linux)
- **ODR violations** для глобальных переменных

**Механика:** shadow memory — на каждые 8 байт приложения выделяется 1 байт теневой памяти, кодирующий доступность. Освобождённые блоки помещаются в **карантин** (не переиспользуются сразу) и помечаются «отравленными» (poisoned) → любое обращение детектируется. Вокруг стековых и глобальных объектов ставятся **redzones** для ловли overflow.

**Цена:** ~2x замедление, ~3x память.

**Отчёт:**

```
==ERROR: AddressSanitizer: heap-use-after-free on address 0x602000000010
READ of size 4 at 0x602000000010 thread T0
    #0 0x... in main prog.cpp:8
freed by thread T0 here:          ← ГДЕ освободили
    #0 0x... in operator delete
    #1 0x... in main prog.cpp:7
previously allocated here:         ← ГДЕ выделили
    #0 0x... in operator new
    #1 0x... in main prog.cpp:6
```

Три стека — использование, освобождение, выделение — это то, за что ASan любят.

**Опции:**

```bash
ASAN_OPTIONS=detect_leaks=1:abort_on_error=1:detect_stack_use_after_return=1 ./prog
```

## UBSan — UndefinedBehaviorSanitizer

```bash
g++ -fsanitize=undefined -g prog.cpp
```

**Что ловит** (ровно наши темы по UB):

- **signed integer overflow** (разбирали)
- некорректные сдвиги (`x << 32`, отрицательный сдвиг)
- деление на ноль, `INT_MIN / -1`
- разыменование nullptr
- misaligned доступ
- некорректный каст (`-fsanitize=vptr` — dynamic_cast к неверному типу)
- выход за границы массива фиксированного размера
- возврат из функции без `return` (non-void)
- преобразования float↔int с потерей

**Цена:** ~1.2-2x — **самый дешёвый**, можно включать шире.

**Полезные подмножества:**

```bash
-fsanitize=signed-integer-overflow    # только знаковое переполнение
-fsanitize=bounds                     # границы массивов
-fsanitize=null                       # nullptr-разыменование
-fsanitize=undefined -fno-sanitize-recover=all   # падать при первой ошибке (CI)
```

По умолчанию UBSan **продолжает** выполнение после обнаружения (печатает и идёт дальше) — для CI нужен `-fno-sanitize-recover=all`.

## TSan — ThreadSanitizer

```bash
g++ -fsanitize=thread -g prog.cpp
```

**Что ловит** (то, что разбирали в concurrency):

- **data races**
- deadlock (частично, для мьютексов с известным порядком)
- некорректное использование мьютексов (unlock чужого, двойной lock)
- гонки на `std::atomic` с неверным memory_order (частично)

**Механика:** отслеживает **happens-before** отношения между обращениями к памяти (векторные часы). Если два обращения к одному адресу не упорядочены happens-before и хотя бы одно — запись → data race.

**Цена:** ~5-15x замедление, ~5-10x память — самый дорогой.

**Ограничение:** ловит только на **выполненных путях** — нужны хорошие многопоточные тесты со стрессом. Гонка, которая не сработала в конкретном прогоне, не будет найдена (но TSan детектирует и _потенциальные_ гонки, не требуя реального одновременного доступа — это его сила по сравнению с «поймать в проде»).

## MSan — MemorySanitizer (только Clang)

```bash
clang++ -fsanitize=memory -fPIE -pie -g prog.cpp
```

Ловит **чтение неинициализированной памяти**. Требует, чтобы **вся** программа, включая libc++, была инструментирована — иначе ложные срабатывания. Поэтому используется реже.

## Совместимость

| |ASan|UBSan|TSan|MSan|
|---|---|---|---|---|
|ASan|—|✅|❌|❌|
|TSan|❌|✅|—|❌|

```bash
g++ -fsanitize=address,undefined ...   # ✅ типичная комбинация
g++ -fsanitize=thread,undefined ...    # ✅
g++ -fsanitize=address,thread ...      # ❌ несовместимы
```

**Практика CI:** две конфигурации — ASan+UBSan для функциональных тестов, TSan для многопоточных.

## Санитайзеры в CMake

```cmake
option(ENABLE_SANITIZERS "Enable sanitizers" OFF)

if(ENABLE_SANITIZERS)
    target_compile_options(app PRIVATE 
        -fsanitize=address,undefined 
        -fno-omit-frame-pointer 
        -fno-sanitize-recover=all)
    target_link_options(app PRIVATE -fsanitize=address,undefined)
endif()
```

**Критично:** флаги нужны и при **компиляции**, и при **линковке** (`target_link_options`) — санитайзеры требуют своей рантайм-библиотеки.

Часто оформляют как отдельный интерфейсный таргет:

```cmake
add_library(sanitizers INTERFACE)
target_compile_options(sanitizers INTERFACE -fsanitize=address,undefined)
target_link_options(sanitizers INTERFACE -fsanitize=address,undefined)
target_link_libraries(app PRIVATE sanitizers)
```

## Платформенная поддержка (важно для твоего стека)

- **Linux/GCC/Clang** — полная поддержка всех санитайзеров
- **MSVC** — только **ASan** (`/fsanitize=address`, с VS 2019 16.9). UBSan/TSan нет
- **MSYS2/MinGW** — ASan/UBSan работают в **CLANG64** (LLVM-based); в UCRT64/MINGW64 (GCC) поддержка **ограничена/отсутствует** (нет рантайм-библиотек санитайзеров под MinGW)
- **macOS/Clang** — ASan, UBSan, TSan

Для тебя практический вывод: на Windows санитайзеры лучше запускать через **CLANG64** в MSYS2, через MSVC (только ASan), или в **Docker/WSL2 с Linux** — что у тебя уже настроено.

---

# Valgrind

**Принципиально другой подход:** не инструментация при компиляции, а **динамическая бинарная трансляция** — valgrind запускает программу на своей виртуальной машине, перехватывая каждое обращение к памяти.

```bash
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./prog
```

## Инструменты valgrind

```bash
valgrind --tool=memcheck ./prog     # по умолчанию — ошибки памяти
valgrind --tool=helgrind ./prog     # data races (аналог TSan)
valgrind --tool=drd ./prog          # тоже гонки, другой алгоритм
valgrind --tool=callgrind ./prog    # профилирование вызовов (+ kcachegrind для GUI)
valgrind --tool=massif ./prog       # профилирование heap-памяти
valgrind --tool=cachegrind ./prog   # симуляция кэша, промахи
```

**Memcheck** ловит:

- утечки памяти (**детальнее ASan**: definitely/indirectly/possibly lost, still reachable)
- чтение неинициализированной памяти (**лучше**, чем у ASan — это сильная сторона)
- use-after-free, double-free
- buffer overflow (хуже, чем ASan — нет redzones вокруг стековых переменных)
- invalid free, mismatched free (`new[]`/`delete`)

## ASan vs Valgrind

| |ASan|Valgrind (memcheck)|
|---|---|---|
|Требует перекомпиляции|✅ да|❌ **нет**|
|Замедление|~2x|**~10-50x**|
|Stack buffer overflow|✅ **отлично** (redzones)|❌ слабо|
|Global overflow|✅|❌|
|Use-after-return|✅ (с опцией)|❌|
|Неинициализированная память|❌ (нужен MSan)|✅ **отлично**|
|Детализация утечек|базовая|✅ **лучше** (категории)|
|Требует debug-символов|для стеков|для стеков|
|Windows|частично (MSVC ASan)|❌ **нет**|
|Поддержка сторонних .so|✅ (перехват malloc)|✅|

**Практика:**

- **ASan — основной** инструмент (быстрее, точнее для overflow, интегрируется в CI)
- **Valgrind — когда** нельзя перекомпилировать (чужой бинарник), нужна детальная картина утечек, или подозрение на неинициализированную память
- Valgrind **не работает на Windows** — только Linux/macOS/BSD (для тебя: через WSL2/Docker)

Многие проекты используют оба: ASan в CI на каждый PR (быстро), valgrind — периодически/на релизах (глубже по утечкам).

## Подавление ложных срабатываний

```bash
valgrind --suppressions=my.supp ./prog
valgrind --gen-suppressions=all ./prog   # сгенерировать заготовки
```

Для ASan:

```bash
ASAN_OPTIONS=suppressions=asan.supp ./prog
LSAN_OPTIONS=suppressions=lsan.supp ./prog   # для утечек
```

Нужно для сторонних библиотек с «известными» утечками (или намеренно не освобождающих в конце).

---

## Формулировки на собеседовании

**«Какие санитайзеры знаешь и что ловят?»** — ASan (use-after-free, buffer overflow, leaks, ODR globals), UBSan (signed overflow, сдвиги, null, misaligned), TSan (data races), MSan (неинициализированная память, Clang-only).

**«ASan vs Valgrind?»** — ASan — инструментация при **компиляции**: быстрее (~2x против ~20x), лучше ловит stack/global overflow и use-after-return, требует пересборки. Valgrind — динамическая трансляция: **не требует пересборки**, лучше ловит неинициализированную память и детальнее по утечкам, но медленный и только Unix.

**«Как работает ASan?»** — Shadow memory (1 байт тени на 8 байт памяти) + redzones вокруг объектов + карантин для освобождённых блоков. Любое обращение к «отравленной» памяти детектируется.

**«Как работает TSan?»** — Отслеживает **happens-before** через векторные часы. Два обращения к одному адресу без happens-before между ними, где хотя бы одно — запись → data race.

**«Какие санитайзеры совместимы?»** — ASan+UBSan и TSan+UBSan — да. ASan+TSan — **нет** (оба инструментируют память по-разному). Отсюда типичный CI: две конфигурации.

**«Флаги для санитайзеров в CMake?»** — И `target_compile_options`, и **`target_link_options`** — санитайзеры требуют рантайм-библиотеки при линковке. Забыть link_options — частая ошибка (undefined reference на `__asan_*`).

**«Санитайзеры на Windows?»** — MSVC — только ASan (`/fsanitize=address`). MinGW/UCRT64 — поддержки почти нет; CLANG64 в MSYS2 — работает ASan/UBSan. Valgrind на Windows отсутствует вообще → WSL2/Docker.

---

Отличие от Java: там весь этот класс инструментов **не нужен** для memory safety — GC исключает use-after-free, double-free и buffer overflow (проверка границ в рантайме → `ArrayIndexOutOfBoundsException`). Аналоги существуют для **других** задач: профилировщики утечек (VisualVM, JFR — но там «утечка» = логическая, объект достижим, хотя не нужен), детекторы гонок (Java нет встроенного TSan-аналога; есть ThreadSanitizer-подобные инструменты в статических анализаторах и `-XX:+PrintConcurrentLocks`). Data races в Java — реальная проблема (JMM их допускает, просто без UB), но инструментария уровня TSan нет — в основном код-ревью и тесты. Итог: C++ требует богатого рантайм-инструментария (санитайзеры, valgrind), потому что язык допускает UB; Java переносит проверки в рантайм самой платформы. Для тебя, поскольку среда уже включает Docker с WSL2 и MSYS2 CLANG64 — практический совет: держи Linux-контейнер для полного набора санитайзеров + valgrind, это закрывает пробелы Windows-тулчейна, и на собеседовании конкретика вроде «ASan+UBSan в CI, TSan отдельной конфигурацией, valgrind для глубокого анализа утечек» звучит как реальный опыт.
