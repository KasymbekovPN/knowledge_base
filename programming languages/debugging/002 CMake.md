---
tags:
  - programming-language
  - debug
  - gdb
  - lldb
---
[[programming languages/debuging/_|<=]]

# Debug / Release / RelWithDebInfo в CMake

## Четыре стандартных `CMAKE_BUILD_TYPE`

|Build type|Флаги (GCC/Clang по умолчанию)|Назначение|
|---|---|---|
|`Debug`|`-g -O0`|Максимальная отладочная точность, без оптимизаций|
|`Release`|`-O3 -DNDEBUG`|Максимальная производительность, отладочной инфы нет вообще|
|`RelWithDebInfo`|`-O2 -g -DNDEBUG`|Продакшн-оптимизации + отладочная информация|
|`MinSizeRel`|`-Os -DNDEBUG`|Минимальный размер бинарника|

Важный нюанс: **`-DNDEBUG`** отключает `assert()` — это не про отладчик напрямую, но часто ловит людей врасплох: в `RelWithDebInfo` твои `assert()`-ы молча исчезают, хотя debug-символы на месте.

## Зачем вообще нужен `RelWithDebInfo`

Это ответ на классическую проблему: **баг воспроизводится только в Release, но в Release нечего отлаживать**. Причины, почему баг "пропадает" в Debug:

- **Race conditions** — на `-O0` код работает медленнее, гонки данных могут не успевать проявиться
- **UB, который "работает" на `-O0`** — неинициализированная память, use-after-free, которые оптимизатор Release превращает в реальный краш
- **Разница в поведении корутин** — как мы недавно обсуждали, некоторые баги в coroutine frame проявляются именно при инлайнинге, которого нет на `-O0`
- **Оптимизации меняют memory layout**, из-за чего heap corruption может "случайно" не задевать ничего важного на `-O0`, но крашить на `-O2`

`RelWithDebInfo` даёт тебе тот же машинный код, что и в проде, но с возможностью посмотреть backtrace, переменные (насколько они выжили после оптимизации) и строки исходника.

## Что реально теряется в отладке на `RelWithDebInfo` (`-O2`)

Продолжая наш эксперимент с корутиной — на `-O2` разница с `-Og` будет куда драматичнее:

- **`<optimized out>`** для гораздо большего числа локальных переменных — не только служебных полей корутины, но и твоих собственных, если они закэшированы в регистр и компилятор посчитал, что хранить их отдельно незачем
- **Переупорядочивание инструкций** — `step`/`next` может "прыгать" не туда, куда ты ожидаешь по тексту кода, потому что реальный порядок исполнения не совпадает с порядком строк
- **Агрессивный инлайнинг** — целые функции пропадают из backtrace, GDB может вообще не показать `[inlined]` пометку (в отличие от LLDB, который эту информацию отражает честнее)
- **Vectorization (SIMD)** — циклы могут быть развёрнуты и векторизованы так, что breakpoint на конкретной строке цикла попадёт в неожиданное место или сработает пачкой сразу для нескольких итераций

## Живой пример: breakpoint не срабатывает на `-O2`

Код (`main.cpp`):

```cpp
int main() {
    const int SIZE{3};
    for (int i{}; i < SIZE; ++i) {
        int sq{i * i};
        std::cout << std::format("sq: {}\n", sq);  // line 8, сюда ставим breakpoint
    }
    return 0;
}
```

Тулчейн: `clang++` из LLVM, таргет `x86_64-pc-windows-msvc`, отладчик — MinGW-gdb (`msys64/clang64/bin/gdb.exe`).

**Предварительная проблема тулчейна.** Clang на MSVC-таргете по умолчанию пишет debug-инфу в CodeView/PDB, а MinGW-gdb понимает только DWARF — в итоге `gdb` говорил `(No debugging symbols found in ./build/debug/app.exe)`, хотя `app.pdb` лежал рядом. Фикс — форсировать DWARF через пресеты:

```json
{
  "name": "debug",
  "cacheVariables": {
    "CMAKE_BUILD_TYPE": "Debug",
    "CMAKE_CXX_FLAGS_DEBUG": "-g -gdwarf-4 -O0"
  }
},
{
  "name": "relwithdebinfo",
  "cacheVariables": {
    "CMAKE_BUILD_TYPE": "RelWithDebInfo",
    "CMAKE_CXX_FLAGS_RELWITHDEBINFO": "-O2 -g -gdwarf-4 -DNDEBUG"
  }
}
```

После пересборки линковщик (`lld-link`) реально кладёт в `.exe` секции `.debug_info`, `.debug_line` и т.д. — то есть DWARF на месте в обеих конфигурациях.

**Debug (`-O0`) — работает штатно:**

```
(gdb) break main.cpp:8
Breakpoint 1 at 0x14000103c: file ...main.cpp, line 8.
(gdb) run
Thread 1 hit Breakpoint 1, main () at ...main.cpp:8
8	        std::cout << std::format("sq: {}\n", sq);
(gdb) print i
$1 = 0
(gdb) continue
Thread 1 hit Breakpoint 1, main () at ...main.cpp:8
(gdb) print sq
$2 = 1
```

Каждая итерация цикла честно останавливает выполнение, `i`/`sq` читаются корректно.

**RelWithDebInfo (`-O2`) — breakpoint резолвится, но не срабатывает ни разу:**

```
(gdb) break main.cpp:8
Breakpoint 1 at 0x14000121a: file ...main.cpp, line 8.
(gdb) run
[New Thread ...]
sq: 0
sq: 1
sq: 4
[Inferior 1 (process ...) exited normally]
(gdb) print i
No symbol "i" in current context.
```

Символы читаются правильно (адрес и строка резолвятся), но программа проходит через выделенный адрес насквозь — ровно та ситуация из пункта про **vectorization/переупорядочивание инструкций** выше: на `-O2` компилятор развернул/слил тело цикла так, что по факту исполняемый путь не проходит через ту машинную инструкцию, на которую resolve'ился breakpoint как "начало строки 8".

Вывод: `RelWithDebInfo` даёт тебе символы и backtrace, но не гарантирует, что breakpoint на конкретной строке внутри горячего цикла вообще остановит выполнение — при отладке оптимизированного кода надёжнее ставить breakpoint на границы функций/базовых блоков, а не на произвольную строку внутри цикла.

## Как задать в CMake

### Однопроходная генерация (классика)

```cmake
set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
```

или из командной строки:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build
```

### Multi-config генераторы (Ninja Multi-Config, Visual Studio)

Тут `CMAKE_BUILD_TYPE` не работает — конфигурация выбирается на этапе сборки:

```bash
cmake -B build -G "Ninja Multi-Config"
cmake --build build --config RelWithDebInfo
```

### Через presets

```json
{
  "configurePresets": [
    {
      "name": "relwithdebinfo",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "RelWithDebInfo"
      }
    }
  ],
  "buildPresets": [
    {
      "name": "relwithdebinfo",
      "configurePreset": "relwithdebinfo"
    }
  ]
}
```

### Явный контроль флагов (когда дефолтные не устраивают)

Если хочешь `RelWithDebInfo`, но с `-Og` вместо `-O2` (компромисс между скоростью и отлаживаемостью):

```cmake
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Og -g -DNDEBUG")
```

## Практическая рекомендация для твоего workflow

Учитывая, что ты работаешь с многопоточным Boost.Asio кодом и корутинами:

1. **Локальная разработка** — `Debug` (`-O0`), максимальная точность отладки
2. **CI / воспроизведение багов, специфичных для прода** — `RelWithDebInfo`, обязательно с `-g3 -gdwarf-5`, чтобы не терять и макросы тоже
3. **Никогда не деплой `Debug` в прод** — не только из-за скорости, но и потому что `-O0` иногда **скрывает** реальные баги (UB), которые потом стреляют в `Release` без предупреждения

## Docker-специфика (раз ты недавно собирал multi-stage Alpine images)

В multi-stage сборке удобно держать **два выходных артефакта**:

```dockerfile
# builder stage
RUN cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo && cmake --build build

# Финальный образ — обрезаем debug-инфу для размера
RUN objcopy --only-keep-debug app app.debug && \
    objcopy --strip-debug app && \
    objcopy --add-gnu-debuglink=app.debug app
```

Так прод-образ маленький (`app` без debug-секций), а `app.debug` хранишь отдельно (артефакт CI) — если баг стрельнёт в проде, подключаешь его через `gdb app` → `symbol-file app.debug`, не пересобирая ничего.
