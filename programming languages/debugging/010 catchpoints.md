---
tags:
  - programming-language
  - debug
  - gdb
  - lldb
---
[[programming languages/debuging/_|<=]]

# Catchpoints — исключения, syscalls, throw/catch

## Что это

Catchpoint — разновидность breakpoint'а, которая срабатывает не на строке кода, а на **событии рантайма**: выброс исключения, вызов `fork()`/`exec()`, загрузка библиотеки, syscall и т.п. Особенно полезно для C++ исключений — когда важно поймать сам момент `throw`, а не гадать, в какой строке кода это происходит.

## C++ исключения

### GDB

```
(gdb) catch throw           # остановка на ЛЮБОМ throw
(gdb) catch catch           # остановка в момент входа в catch-блок
(gdb) catch rethrow         # остановка на re-throw (throw; без аргумента)
```

Фильтр по конкретному типу исключения:

```
(gdb) catch throw std::runtime_error
(gdb) catch throw if $_exception_type == "MyCustomException"
```

### LLDB

```
(lldb) breakpoint set --name __cxa_throw          # низкоуровневый способ через libstdc++ ABI
```

У LLDB **нет полноценного `catch throw` в GDB-стиле** для C++ — вместо специальной catchpoint-команды используется breakpoint на внутреннюю функцию рантайма `__cxa_throw` (это точка входа "выбросить исключение" в Itanium C++ ABI, который использует и GCC, и Clang на Linux).

## Syscalls (только Linux, GDB)

```
(gdb) catch syscall open
(gdb) catch syscall read write
(gdb) catch syscall            # ЛЮБОЙ syscall — очень болтливо, используй с осторожностью
```

## Важный нюанс: LLDB не имеет нативных syscall catchpoints на Linux

В отличие от C++ исключений (где есть обходной путь через `__cxa_throw`), для syscalls в LLDB на Linux аналога `catch syscall` нет вообще. Альтернативы:
- `strace -e trace=write ./binary` — отдельный инструмент, не встроен в LLDB
- Breakpoint на конкретную libc-обёртку (`breakpoint set --name write`) — работает, но это не то же самое, что перехват самого syscall на уровне ядра

## Сводная таблица

| Событие | GDB | LLDB |
|---|---|---|
| `throw` | `catch throw` (родная catchpoint) | breakpoint на `__cxa_throw` (обходной путь) |
| `catch`-блок | `catch catch` | нет прямого аналога |
| `rethrow` | `catch rethrow` | нет прямого аналога |
| syscall | `catch syscall <name>` | нет (используй `strace`) |
| fork/exec | `catch fork`, `catch exec` | `breakpoint set --name fork` (обходной путь) |
| загрузка библиотеки | `catch load`, `catch unload` | `breakpoint set -A -s <lib.so>` через `--shlib` |

## Практический вывод

Для C++ exception-debugging **GDB эргономичнее из коробки** (`catch throw` — одна команда, работает сразу). LLDB требует знания внутреннего ABI (`__cxa_throw`) — тот случай, когда "родная" LLDB-философия объект.метод не даёт готового glagola под конкретно это событие, и приходится опускаться на уровень ниже.

## Как собрать и повторить эксперимент

```bash
g++ -std=c++20 -g exceptions_demo.cpp -o exceptions_demo
```

**GDB — поймать оба throw:**

```bash
gdb --args ./exceptions_demo
(gdb) catch throw
(gdb) run
(gdb) bt
(gdb) continue
(gdb) bt
```

**LLDB — через breakpoint на `__cxa_throw`:**

```bash
lldb -- ./exceptions_demo
(lldb) breakpoint set --name __cxa_throw
(lldb) run
(lldb) bt
(lldb) continue
(lldb) bt
```

Точки останова в коде для справки:

- **строка 7** (`throw std::invalid_argument(...)`)
- **строка 13** (`throw std::runtime_error(...)`)
- **строка 27** (`catch (const std::invalid_argument& e)`) — можно попробовать `catch catch` в GDB, чтобы поймать именно момент входа в catch-блок

Хочешь, добавлю третью функцию с **вложенными** try/catch или с исключением, вылетающим из деструктора — частый сценарий багов, где `catch throw` особенно выручает?

---

```cpp
#include <iostream>
#include <stdexcept>
#include <string>

void validate(int value) {
    if (value < 0) {
        throw std::invalid_argument("value must be non-negative: " + std::to_string(value));
    }
}

int risky_divide(int a, int b) {
    if (b == 0) {
        throw std::runtime_error("division by zero");
    }
    return a / b;
}

int main() {
    try {
        validate(5);
        std::cout << "5 is valid\n";

        validate(-3);                    // <-- здесь бросится invalid_argument
        std::cout << "unreachable\n";
    } catch (const std::invalid_argument& e) {
        std::cout << "Caught invalid_argument: " << e.what() << "\n";
    }

    try {
        int result = risky_divide(10, 0);  // <-- здесь бросится runtime_error
        std::cout << "result = " << result << "\n";
    } catch (const std::runtime_error& e) {
        std::cout << "Caught runtime_error: " << e.what() << "\n";
    }

    std::cout << "Done\n";
    return 0;
}
```


----

##  Пример

### main.cpp
```cpp
!!!
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
                "CMAKE_CXX_FLAGS_DEBUG": "-g -gdwarf-4 -O0"  
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



---
---
---
----
---


## Этап 2. Точки останова и управление выполнением (2-3 дня)

- Step/next/finish/until — тонкости step into vs step over при инлайнинге
- Temporary breakpoints, disable/enable, breakpoint commands (авто-выполнение команд при остановке)
- Работа с `tbreak`, `rbreak` (regex breakpoints)

## Этап 3. Исследование состояния программы (2-3 дня)

- Просмотр переменных: `print`, `display`, форматированный вывод (`/x`, `/t`, `/o`)
- Просмотр структур, массивов, указателей, разыменование
- Backtrace: `bt`, `bt full`, навигация по фреймам (`frame`, `up`, `down`)
- Просмотр регистров и памяти напрямую (`x/10xw $rsp`)
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