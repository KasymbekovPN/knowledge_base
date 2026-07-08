---
tags:
  - programming-language
  - debug
  - gdb
  - lldb
---
[[programming languages/debugging/_|<=]]

# Просмотр регистров и памяти напрямую

## Регистры

### GDB

```
(gdb) info registers          # все регистры общего назначения
(gdb) info registers rax       # конкретный регистр
(gdb) print $rip                # текущий instruction pointer
(gdb) print $rsp                # stack pointer
(gdb) print $rbp                # frame pointer (base pointer)
(gdb) print/x $rax              # в hex явно
(gdb) info all-registers        # включая FPU/SSE/AVX регистры
```

Любой регистр доступен через `$` как обычная переменная — можно использовать в выражениях: `print $rsp + 8`.

### LLDB

```
(lldb) register read              # все регистры
(lldb) register read rax rbx       # конкретные
(lldb) register read --all         # включая FPU/vector
(lldb) print $rip
(lldb) print $rsp
```

## Команда `x` — просмотр памяти

Синтаксис: **`x/NFU адрес`**

- **N** — сколько единиц показать
- **F** — формат (`x`=hex, `d`=decimal, `t`=binary, `o`=octal, `c`=char, `s`=string, `i`=инструкция!)
- **U** — размер единицы (`b`=byte, `h`=halfword/2 байта, `w`=word/4 байта, `g`=giant/8 байт)Обрати внимание на последнюю команду: **`x/3i $rip`** — команда `x` умеет не только hex-дампить память, но и **дизассемблировать** прямо с текущего instruction pointer (`i` — формат "инструкция"). Видно реальный машинный код строки `return squared + doubled;` — два `mov` (загрузка `squared`/`doubled` из стека) + `add`.

## GDB-style алиас `x` тоже работает в LLDB

Да, GDB-style `x/10xw $rsp` тоже работает в LLDB через алиас — идентичный вывод.

## Сводная таблица

|Действие|GDB|LLDB (родной)|LLDB (алиас)|
|---|---|---|---|
|Все регистры|`info registers`|`register read`|—|
|Конкретный регистр|`print $rax`|`register read rax`|`print $rax`|
|Память как hex/word|`x/10xw $rsp`|`memory read --size 4 --format x --count 10 $rsp`|`x/10xw $rsp`|
|Память как 8-байтовые слова|`x/4xg $rsp`|`memory read --size 8 --format x --count 4 $rsp`|`x/4xg $rsp`|
|Дизассемблировать с адреса|`x/3i $rip`|`disassemble --start-address $rip --count 3`|`x/3i $rip`|
|Строка в памяти|`x/s ptr`|`memory read --format s`|`x/s ptr`|
|Побайтово с ASCII|`x/20cb ptr`|`memory read --size 1 --format c --count 20`|`x/20cb ptr`|

## Практическое применение (актуально для тебя с Boost.Asio/сетевым кодом)

`x/Ns` (просмотр памяти как строки) особенно полезен при отладке **сырых буферов** (`char* buffer`, `std::byte*`) — например, посмотреть, что реально прилетело в TCP-сокет до парсинга, как в твоём `socketbuf`:

````
(gdb) x/64xb buffer.data()    # первые 64 байта сырого буфера в hex
(gdb) x/s buffer.data()        # если ожидается текстовый протокол
```Файл `frames_demo.cpp` уже был отправлен ранее — этот же пример подошёл и для регистров/памяти, дополнительный файл не нужен.

Хочешь дальше — **работа с STL-контейнерами: pretty-printers для GDB (Python-based) и data formatters LLDB**, следующий (и последний) пункт Этапа 3?
````

## Пример

### main.cpp
```cpp
#include <iostream>  
#include <format>  
#include <string>  
  
int level_three(const int x) {  
    const int squared{x * x};  
    const int doubled{2 * x};  
  
    return squared + doubled; // <-- breakpoint здесь, самый глубокий фрейм  
}  
  
int level_two(const int a, const int b) {  
    std::string label{"processing level_two"};  
    const int sum{a + b};  
    const int result{level_three(sum)};  
  
    return 2 * result;  
}  
  
int level_one(const int input) {  
    std::string context{"top-level call"};  
    const int adjusted{10 + input};  
    const int final_result{level_two(adjusted, input)};  
  
    return final_result;  
}  
  
int main() {  
    constexpr int value{5};  
    const int answer{level_one(value)};  
  
    std::cout << std::format("answer: {}\n", answer);  
  
    return 0;  
}  
  
/*  
  
###  
lldb .\build\debug\app.exe  
breakpoint set --file main.cpp --line 9  
run  
register read  
register read rax rbx  
register read --all  
print $rip  
print $rsp  
  
###  
C:\msys64\clang64\bin\gdb.exe .\build\debug\app.exe  
break main.cpp:9  
run  
info registers  
info registers rax  
print $rip  
print $rsp  
print $rbp  
print/x $rax  
info all-registers  
  
*/
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.20)  
project(proj LANGUAGES CXX)  
  
add_executable(app main.cpp)  
target_compile_features(app PUBLIC cxx_std_20)
```

### CMakePresets.json
```json
{  
    "version": 6,  
    "configurePresets": [  
        {
			"name": "base",  
            "hidden": true,  
            "generator": "Ninja",  
            "binaryDir": "${sourceDir}/build/${presetName}",  
            "toolchainFile": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"  
        },  
        {
			"name": "debug",  
            "inherits": "base",  
            "cacheVariables": {  
                "CMAKE_BUILD_TYPE": "Debug",  
                "CMAKE_CXX_FLAGS_DEBUG": "-g3 -gdwarf-5 -O0"  
            }  
        }
	],
	"buildPresets": [  
        {
			"name": "debug",  
            "configurePreset": "debug"  
        }  
    ]
}
```

### vcpkg.json
```json
{  
    "name": "demo",  
    "version": "0.1.0",  
    "dependencies": []  
}
```



----
---
---
----
----
---
---
---
---
----
---


## Этап 3. Исследование состояния программы (2-3 дня)

- Работа с STL-контейнерами — pretty-printers для GDB (Python-based) и встроенная поддержка LLDB (data formatters)

## Этап 4. Отладка многопоточного и асинхронного кода (3-4 дня) — _с прицелом на твой опыт с Asio/корутинами_

- Команды для потоков: `info threads`, `thread apply all bt`, переключение потоков
- Non-stop mode и scheduler-locking в GDB
- Отладка гонок данных, deadlock через анализ стеков всех потоков
- Особенности отладки корутин C++20: как читать состояние `promise_type`, проблемы с оптимизированными фреймами корутин, символы для coroutine frame
- Отладка callback-based кода Boost.Asio: точки останова внутри lambda, работа с io_context

## Этап 5. Python API в GDB и Python scripting в LLDB (2-3 дня)

- Написание кастомных pretty-printers для своих классов (GDB Python API)
- Автоматизация: скрипты для повторяющихся сценариев отладки
- LLDB Python scripting bridge (`script`, кастомные команды)
- Написание `.gdbinit` / `.lldbinit` с полезными алиасами и автозагрузкой printers

## Этап 6. Продвинутая отладка (3-4 дня)

- Core dumps: генерация, анализ (`gdb ./binary core`, `lldb -c core`)
- Reverse debugging в GDB (`record`, `reverse-step`, `reverse-continue`)
- Отладка оптимизированного кода (`-O2`), проблемы с инлайнингом и переменными "optimized out"
- ASan/UBSan интеграция с отладчиком, анализ crash-репортов
- Отладка shared libraries, символы (`.debug` файлы, `objcopy --only-keep-debug`, символьные серверы)
- Remote debugging (`gdbserver`, `lldb-server`) — актуально для отладки внутри Docker/Alpine контейнеров

## Этап 7. Интеграция с инструментами (1-2 дня)

- GDB/LLDB внутри VS Code и CLion — конфигурация `launch.json`, интеграция с CMake presets
- `rr` (record & replay) как надстройка над GDB для детерминированной отладки
- TUI-режим GDB (`gdb -tui`) и его аналоги

---

## Формат работы

Как и в прошлых темах (CMake, Boost, корутины) — предлагаю двигаться блоками с практическими примерами: на каждом этапе будем брать конкретный кусок кода (например, твой `socketbuf` или producer/consumer с корутинами) и разбирать его отладку вживую.

Хочешь начать с Этапа 1, или есть тема, с которой хочешь стартовать сразу (например, отладка корутин — раз ты недавно ей глубоко занимался)?