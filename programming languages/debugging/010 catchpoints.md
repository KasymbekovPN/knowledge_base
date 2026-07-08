---
tags:
  - programming-language
  - debug
  - gdb
  - lldb
---
[[programming languages/debugging/_|<=]]

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

## Важный нюанс: `__cxa_throw` работает только для Itanium C++ ABI

`__cxa_throw` и `catch throw` (в её обычной реализации) завязаны на **Itanium C++ ABI** — это GCC/Clang с libstdc++ или libc++ (Linux, а также clang64-окружение MSYS2 на Windows). Если бинарник собран **MSVC** (`cl.exe`, в том числе через vcpkg-toolchain из CMakePresets), то `__cxa_throw` в нём **не существует вообще** — MSVC использует собственный SEH-based механизм, и точка входа для throw называется **`_CxxThrowException`** (экспортируется из `vcruntime140.dll` / `vcruntime140d.dll` в debug-сборке).

Как отличить, что перед тобой MSVC-бинарник, если не уверен, чем он собран: посмотри загруженные модули (`info sharedlibrary` в gdb после `start`, или `target modules list` в lldb после `run`) — если видишь `vcruntime140d.dll`, `msvcp140d.dll`, `ucrtbased.dll`, это MSVC. `libc++.dll`/`libstdc++-6.dll` — это Itanium ABI.

Для MSVC-бинарника рабочий рецепт:

```
(gdb) set breakpoint pending on
(gdb) break _CxxThrowException
(gdb) run
(gdb) bt
```

```
(lldb) breakpoint set --name _CxxThrowException
(lldb) run
(lldb) bt
```

Оба покажут `no locations (pending)` при установке — это нормально, символ появляется только после загрузки `vcruntime140d.dll`, что происходит рано в рантайме (до входа в `main`), так что pending благополучно резолвится сам.

**Важно для lldb:** если гонишь команды через `stdin` (pipe), может проскочить гонка между резолвом pending breakpoint и командой `bt` — `bt` тогда падает с "Command requires a process which is currently stopped", хотя брейкпоинт по факту сработал бы. Используй `lldb -s script.txt` (командный файл) вместо пайпа, чтобы команды шли строго синхронно.

## Сводная таблица

| Событие | GDB | LLDB |
|---|---|---|
| `throw` | `catch throw` (родная catchpoint, только Itanium ABI) | breakpoint на `__cxa_throw` (Itanium ABI) или `_CxxThrowException` (MSVC) |
| `catch`-блок | `catch catch` | нет прямого аналога |
| `rethrow` | `catch rethrow` | нет прямого аналога |
| syscall | `catch syscall <name>` | нет (используй `strace`) |
| fork/exec | `catch fork`, `catch exec` | `breakpoint set --name fork` (обходной путь) |
| загрузка библиотеки | `catch load`, `catch unload` | `breakpoint set -A -s <lib.so>` через `--shlib` |

## Практический вывод

Для C++ exception-debugging **GDB эргономичнее из коробки** (`catch throw` — одна команда, работает сразу). LLDB требует знания внутреннего ABI (`__cxa_throw`) — тот случай, когда "родная" LLDB-философия объект.метод не даёт готового glagola под конкретно это событие, и приходится опускаться на уровень ниже.

##  Пример

### main.cpp
```cpp
#include <iostream>  
#include <format>  
#include <stdexcept>  
#include <string>  
  
void validate(int value) {  
    if (value >= 0) return;  
    throw std::invalid_argument(std::format("value must be non-negative: {}", value));  
}  
  
int risky_divide(int a, int b) {  
    if (b == 0) {  
        throw std::runtime_error("division by zero");  
    }    return a / b;  
}  
  
int main() {  
    try {  
        validate(5);  
        validate(-6);  
    } catch (const std::invalid_argument& e) {  
        std::cout << std::format("Caught invalid_argument: {}\n", e.what());  
    }  
    try {  
        risky_divide(5, 0);  
    } catch (const std::runtime_error& e)  {  
        std::cout << std::format("Caught runtime_error: {}\n", e.what());  
    }  
    std::cout << "Done\n";  
    return 0;  
}  
  
/*  
  
###  
lldb .\build\debug\app.exe  
breakpoint set --name _CxxThrowException  
run  
bt  
continue  
bt  
  
###  
C:\msys64\clang64\bin\gdb.exe .\build\debug\app.exe  
set breakpoint pending on  
break _CxxThrowException  
run  
bt  
continue  
bt  
  
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
