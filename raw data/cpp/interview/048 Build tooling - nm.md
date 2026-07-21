[[raw data/cpp/interview/_|<=]]

# `nm` и Windows-аналоги

## `nm` — таблица символов объектного файла

Показывает символы (функции, переменные), которые объектный файл/библиотека **определяет** или **требует извне**.

```bash
nm main.o
```

```
0000000000000000 T _Z3fooi
                 U _Z6helperv
0000000000000000 B counter
                 U _ZSt4cout
```

Формат: `[адрес] [тип] [имя]`

---

## Типы символов — ключевая таблица

**Заглавная буква = глобальный (external) символ, строчная = локальный (static/internal linkage).**

|Символ|Значение|
|---|---|
|**T** / t|`.text` — **определённая функция** (код)|
|**U**|**Undefined** — требуется извне (не определён здесь)|
|**D** / d|`.data` — инициализированная глобальная переменная|
|**B** / b|`.bss` — неинициализированная глобальная (зануляется)|
|**R** / r|`.rodata` — константа (const, строковые литералы, **vtable**)|
|**W** / w|**Weak** — слабый символ; может быть переопределён|
|**V** / v|Weak object (аналогично W, для данных)|
|**C**|Common — неинициализированная (C-стиль tentative definition)|
|**I**|Indirect|
|**A**|Absolute — адрес не меняется при релокации|
|**N**|Debug-символ|
|**?**|Неизвестный тип|

### Практическая интерпретация

```bash
nm -C main.o
```

```
                 U helper()                    ← НУЖЕН, но не определён здесь
0000000000000000 T foo(int)                    ← ОПРЕДЕЛЁН здесь
0000000000000000 W Widget::getValue() const    ← inline/шаблон → weak
0000000000000000 V vtable for Widget           ← vtable (weak, .rodata)
0000000000000004 B globalCounter               ← .bss (неинициализированная)
```

**`W` (weak) — важный случай:** inline-функции, шаблоны, методы в классе получают weak-линковку. Именно поэтому ODR разрешает их множественные определения (мы разбирали) — линкер видит несколько weak-символов и **выбирает один**, вместо ошибки `multiple definition`.

```cpp
inline int f() { return 42; }   // → W (weak) — можно в нескольких TU
int g() { return 42; }           // → T (strong) — только в одной TU
```

---

## Ключевые опции

```bash
nm -C file.o          # ⭐ demangle — человекочитаемые C++ имена
nm -u file.o          # только UNDEFINED (что требуется извне)
nm --defined-only f.o # только определённые
nm -g file.o          # только глобальные (external) символы
nm -D lib.so          # ⭐ динамические символы (для .so — обязательно!)
nm -A *.o             # префиксом показать имя файла (для grep по многим)
nm -n file.o          # сортировать по адресу
nm -S file.o          # показать РАЗМЕР символов
nm --size-sort file.o # сортировать по размеру (найти толстые функции)
```

**Критично:** для `.so` используй `nm -D` — обычный `nm` покажет статическую таблицу (часто пустую после `strip`), а `-D` — динамическую (реально экспортируемые символы).

```bash
nm libmath.so          # может быть "no symbols" (stripped)
nm -D libmath.so       # ✅ динамические символы — то, что видно линкеру
```

---

## Практические рецепты

### 1. Диагностика undefined reference

```bash
# Ошибка: undefined reference to `helper()'

# Шаг 1: кто ТРЕБУЕТ?
nm -C -u main.o | grep helper
#   U helper()                      ← main.o нужен helper

# Шаг 2: кто ОПРЕДЕЛЯЕТ? Ищем по всем объектникам и библиотекам
nm -C --defined-only *.o | grep helper
nm -C --defined-only libutil.a | grep helper
nm -DC --defined-only libutil.so | grep helper
# Пусто → символ нигде не определён (забыли .cpp или библиотеку)
```

### 2. Поиск mangling mismatch

```bash
nm -C -u main.o | grep process
#   U process(int)               ← вызов ждёт (int)

nm -C --defined-only lib.o | grep process
#   T process(long)              ← а определён (long) → НЕ СОВПАДАЕТ
```

Именно это часто скрывается за «функция же есть, почему undefined reference».

### 3. Найти, кто определяет символ, в куче объектников

```bash
nm -AC --defined-only *.o | grep "myFunction"
# util.o: T myFunction()          ← вот кто
```

`-A` добавляет имя файла — незаменимо при поиске по многим файлам.

### 4. Проверить экспорт библиотеки

```bash
nm -DC --defined-only libmath.so | grep " T "     # что библиотека экспортирует
```

### 5. Найти multiple definition

```bash
nm -AC --defined-only *.o | grep " T " | awk '{print $3}' | sort | uniq -d
# выведет символы, определённые в НЕСКОЛЬКИХ .o
```

### 6. Поиск толстых функций

```bash
nm -C --size-sort -S program | tail -20    # 20 самых больших символов
```

Полезно при борьбе с раздутием бинарника (code bloat от шаблонов — мы разбирали мономорфизацию).

---

## Windows-аналоги

### 1. `dumpbin` — основной инструмент MSVC

Поставляется с Visual Studio. Запускать из **Developer Command Prompt** (или после `vcvarsall.bat`).

```cmd
:: символы объектного файла / статической библиотеки
dumpbin /SYMBOLS main.obj
dumpbin /SYMBOLS mathlib.lib

:: ЭКСПОРТЫ DLL (аналог nm -D --defined-only)
dumpbin /EXPORTS mathlib.dll

:: ИМПОРТЫ — что бинарник требует извне (аналог nm -u)
dumpbin /IMPORTS program.exe

:: зависимости DLL (аналог ldd)
dumpbin /DEPENDENTS program.exe

:: заголовки, секции
dumpbin /HEADERS program.exe

:: дизассемблирование (аналог objdump -d)
dumpbin /DISASM main.obj
```

**Соответствие `nm` ↔ `dumpbin`:**

|`nm`|`dumpbin`|
|---|---|
|`nm file.o`|`dumpbin /SYMBOLS file.obj`|
|`nm -D --defined-only lib.so`|`dumpbin /EXPORTS lib.dll`|
|`nm -u file.o`|`dumpbin /IMPORTS file.exe`|
|`ldd program`|`dumpbin /DEPENDENTS program.exe`|
|`objdump -d`|`dumpbin /DISASM`|
|`nm -C` (demangle)|`undname` (отдельная утилита)|

### 2. `undname` — demangling MSVC

MSVC использует **другую** схему mangling (не Itanium ABI, как GCC/Clang):

```cmd
undname ?foo@@YAXH@Z
:: → void __cdecl foo(int)
```

```cpp
// Сравнение mangling:
void foo(int);
// GCC/Clang (Itanium ABI): _Z3fooi
// MSVC:                    ?foo@@YAXH@Z
```

Это одна из причин исторической ABI-несовместимости GCC ↔ MSVC (мы разбирали в ABI) — разные mangling схемы, `c++filt` не понимает MSVC-имена и наоборот.

`dumpbin /SYMBOLS` обычно показывает и mangled, и раскодированное имя рядом.

### 3. MSYS2/MinGW — тот же `nm` (твой стек!)

Раз ты работаешь с **MSYS2/MinGW (UCRT64, CLANG64)**, у тебя доступны **нативные GNU binutils**:

```bash
# в MSYS2 shell:
nm -C main.o
nm -DC --defined-only libmath.dll
objdump -d main.o
c++filt _Z3fooi
ldd program.exe        # ✅ работает в MSYS2 для MinGW-бинарников
readelf                # (для ELF; для PE лучше objdump)
```

MinGW-собранные бинарники используют **Itanium mangling** (как GCC на Linux), поэтому `nm -C` и `c++filt` работают нормально. Это удобно: тот же инструментарий, что на Linux.

**Важно:** MinGW и MSVC генерируют **несовместимые** объектные файлы и разный mangling → нельзя линковать `.obj` от MSVC с `.o` от MinGW (кроме C-интерфейсов).

### 4. Dependency Walker / Dependencies — GUI для зависимостей

```
Dependencies (современная замена Dependency Walker)
https://github.com/lucasg/Dependencies
```

GUI-инструмент: показывает дерево зависимостей DLL, экспорты/импорты, находит отсутствующие DLL. Аналог `ldd` с графическим деревом. Классический Dependency Walker устарел (не понимает API sets Windows 10+), используй **Dependencies**.

### 5. `llvm-nm` — кроссплатформенный

Если установлен LLVM (в том числе через MSYS2 CLANG64):

```bash
llvm-nm main.o          # понимает и ELF, и PE/COFF, и Mach-O
llvm-nm -C main.obj     # demangle (понимает и Itanium, и MSVC mangling!)
llvm-objdump -d main.o
llvm-readobj --symbols main.obj
```

`llvm-nm` — универсальный вариант: работает с объектниками MSVC и GCC, умеет demangle обеих схем. Хороший выбор, когда работаешь на обеих платформах.

---

## Сводная таблица платформ

|Задача|Linux|Windows (MSVC)|Windows (MSYS2/MinGW)|
|---|---|---|---|
|Символы .o|`nm -C`|`dumpbin /SYMBOLS`|`nm -C`|
|Экспорты библиотеки|`nm -DC lib.so`|`dumpbin /EXPORTS lib.dll`|`nm -DC lib.dll`|
|Импорты (undefined)|`nm -u`|`dumpbin /IMPORTS`|`nm -u`|
|Зависимости|`ldd`|`dumpbin /DEPENDENTS`, Dependencies|`ldd`|
|Demangle|`c++filt`|`undname`|`c++filt`|
|Дизассемблер|`objdump -d`|`dumpbin /DISASM`|`objdump -d`|
|Универсальный|—|`llvm-nm`, `llvm-objdump`|`llvm-nm`|

---

## Формулировки на собеседовании

**«Что показывает `nm`?»** — Таблицу символов: что объектный файл/библиотека **определяет** (T/D/B/R/W) и что **требует извне** (U). Основной инструмент диагностики ошибок линковки.

**«Что значат T и U?»** — `T` — функция **определена** в `.text` этого файла. `U` — символ **undefined**, требуется из другой TU/библиотеки. Заглавные — глобальные символы, строчные — локальные (static).

**«Что значит W?»** — Weak symbol: inline-функции, шаблоны, методы в классе. Линкер допускает **несколько** weak-определений и выбирает одно — механизм, реализующий ODR-исключения для inline/шаблонов.

**«Как найти, кто определяет символ?»** — `nm -AC --defined-only *.o | grep symbolName` (`-A` показывает имя файла, `-C` demangle).

**«Почему `nm libfoo.so` пусто?»** — Библиотека stripped (статическая таблица удалена). Нужен `nm -D` — динамическая таблица символов.

**«Windows-аналог?»** — `dumpbin` (`/SYMBOLS`, `/EXPORTS`, `/IMPORTS`, `/DEPENDENTS`) для MSVC + `undname` для demangling (MSVC использует свою схему mangling, не Itanium). В MSYS2/MinGW — нативные GNU binutils (`nm`, `objdump`, `c++filt`). Универсально — `llvm-nm` (понимает оба формата и обе схемы mangling).

---

Раз у тебя развёрнут MSYS2 с UCRT64 и CLANG64, практический совет для подготовки: попробуй на своих проектах прогнать `nm -C` по объектникам и намеренно создать ошибки линковки (несовпадение сигнатуры, забытое определение виртуальной функции → `undefined reference to vtable`), а потом продиагностировать их через `nm`. На собеседовании умение сказать «я бы посмотрел `nm -C -u` на объектнике, чтобы понять, какой mangled символ ищется» — это конкретика, которая отличает практика. Плюс полезно один раз сравнить mangling GCC (`_Z3fooi`) и MSVC (`?foo@@YAXH@Z`) на одном коде — наглядно объясняет, почему ABI между ними несовместим.
