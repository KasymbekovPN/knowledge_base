---
tags:
  - programming-language
  - debug
  - gdb
  - lldb
---
[[programming languages/debugging/_|<=]]

# Запуск GDB/LLDB: аргументы и переменные окружения

## Базовый запуск

```bash
gdb ./binary
lldb ./binary
```

Оба просто загружают бинарник, но **не запускают** его сразу — нужна команда `run`/`r` внутри сессии.

## Передача аргументов командной строки

### GDB

**Способ 1 — через флаг при запуске:**

```bash
gdb --args ./binary arg1 arg2 --flag=value
```

`--args` обязателен, иначе GDB попытается интерпретировать `arg1` как ещё один бинарник/core file.

**Способ 2 — внутри сессии, командой `set args`:**

```bash
gdb ./binary
(gdb) set args arg1 arg2 --flag=value
(gdb) run
```

**Способ 3 — прямо в `run`:**

```bash
(gdb) run arg1 arg2
```

Работает, но `set args` удобнее, если планируешь перезапускать много раз с одними и теми же аргументами.

Посмотреть текущие аргументы:

```bash
(gdb) show args
```

### LLDB

**Способ 1 — при запуске:**

```bash
lldb -- ./binary arg1 arg2 --flag=value
```

`--` здесь обязателен — разделяет опции самого LLDB от аргументов твоей программы.

**Способ 2 — внутри сессии:**

```bash
lldb ./binary
(lldb) settings set target.run-args arg1 arg2 --flag=value
(lldb) run
```

**Способ 3 — прямо в `run`:**

```bash
(lldb) run arg1 arg2
```

## Переменные окружения

### GDB

**Установить одну переменную:**

```bash
(gdb) set environment MY_VAR=value
(gdb) run
```

**Удалить переменную из окружения инферiора (важно для тестирования edge cases):**

```bash
(gdb) unset environment MY_VAR
```

**Посмотреть текущее окружение:**

```bash
(gdb) show environment
(gdb) show environment MY_VAR
```

**Полностью изолированное окружение через bash-обёртку** (если нужно много переменных сразу):

```bash
MY_VAR=value ANOTHER_VAR=42 gdb ./binary
```

GDB унаследует их автоматически — это часто удобнее, чем набирать `set environment` построчно.

### LLDB

**Установить переменную:**

```bash
(lldb) settings set target.env-vars MY_VAR=value ANOTHER_VAR=42
(lldb) run
```

**Посмотреть текущие:**

```bash
(lldb) settings show target.env-vars
```

**Очистить всё окружение и задать с нуля** (LLDB по умолчанию наследует окружение шелла — если нужна полная изоляция):

```bash
(lldb) settings set target.inherit-env false
(lldb) settings set target.env-vars MY_VAR=value
```

## Практический пример под твой контекст (Boost.Asio + переменные окружения для логирования)

Допустим, у тебя есть переменная типа `ASIO_LOG_LEVEL` или кастомный `BOOST_ASIO_DISABLE_THREADS`:

**GDB:**

```bash
gdb --args ./my_server --port 8080 --config prod.json
(gdb) set environment ASIO_LOG_LEVEL=debug
(gdb) set environment BOOST_ASIO_DISABLE_THREADS=1
(gdb) run
```

**LLDB:**

```bash
lldb -- ./my_server --port 8080 --config prod.json
(lldb) settings set target.env-vars ASIO_LOG_LEVEL=debug BOOST_ASIO_DISABLE_THREADS=1
(lldb) run
```

## Working directory программы

Иногда важно, из какой директории программа стартует (относительные пути к конфигам и т.п.):

**GDB:**

```bash
(gdb) cd /path/to/workdir
(gdb) run
```

**LLDB:**

```bash
(lldb) settings set target.run-args ...
(lldb) process launch --working-dir /path/to/workdir
```

либо через `settings set target.working-directory /path/to/workdir` перед `run`.

## Важный нюанс: stdin/stdout redirect

Если программа читает из stdin или пишет в файл:

**GDB:**

```bash
(gdb) run < input.txt > output.txt
```

**LLDB** не поддерживает такой синтаксис напрямую в `run` — используй:

```bash
(lldb) settings set target.input-path input.txt
(lldb) settings set target.output-path output.txt
(lldb) run
```

---

## Пример с `.gdbinit`/`.lldbinit` для повторяемости

Раз у тебя частая практика — собирать/запускать один и тот же бинарник много раз с одинаковыми аргументами, стоит зафиксировать в проектном init-файле:

**`.gdbinit`** (в директории проекта, если `set auto-load safe-path` разрешает):

```
file ./coro_O0
set args --iterations 10
set environment LOG_LEVEL=debug
break coro_demo.cpp:46
```

Запуск после этого — просто `gdb -x .gdbinit`, без ручного набора команд.

**`.lldbinit`:**

```
target create ./coro_O0
settings set target.run-args --iterations 10
settings set target.env-vars LOG_LEVEL=debug
breakpoint set --file coro_demo.cpp --line 46
```

Запуск: `lldb -s .lldbinit` (или автозагрузка при обычном `lldb` из этой директории, если разрешено в конфиге).

## Пример

### main.cpp
```cpp
#include <iostream>  
#include <format>  
#include <cstdlib>  
#include <string>  
  
int main(int argc, char *argv[]) {  
    // --- Аргументы командной строки ---  
    std::cout << std::format("argc = {}\n", argc);  
    for (int i{}; i < argc; ++i) {  
        std::cout << std::format("argv[{}] = {}\n", i, argv[i]);  
    }  
    // --- Переменные окружения ---  
    const char* LOG_LEVEL = std::getenv("LOG_LEVEL");  
    const char* ASIO_DOSABLE_THREADS = std::getenv("BOOST_ASIO_DISABLE_THREADS");  
  
    const std::string slog_level = LOG_LEVEL ? LOG_LEVEL : "NOT SET";  
    const std::string sthreads = ASIO_DOSABLE_THREADS ? ASIO_DOSABLE_THREADS : "NOT SET";  
  
    std::cout << std::format("LOG_LEVEL: {}\n", slog_level);  
    std::cout << std::format("BOOST_ASIO_DISABLE_THREADS: {}\n", sthreads);  
  
    return 0;  
}  
  
/*  
  
cmake --preset release  
cmake --build --preset release  
cmake --build --preset release --config=Release  
  
###  
  
C:\msys64\clang64\bin\gdb.exe --args .\build\debug\app.exe one two --flag=42  
set environment LOG_LEVEL=debug  
set environment BOOST_ASIO_DISABLE_THREADS=1  
break main.cpp:21  
run  
print argv[1]  
print argv[2]  
print argv[3]  
print slog_level  
print s_threads  
  
###  
  
lldb -- .\build\debug\app.exe one two --flag=42  
settings set target.env-vars LOG_LEVEL=debug BOOST_ASIO_DISABLE_THREADS=1  
breakpoint set --file main.cpp --line 21  
run  
print argv[1]  
print argv[2]  
print argv[3]  
print slog_level  
print sthreads  
  
 */
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.20)  
project(Demo LANGUAGES CXX)  
  
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

### .gdbinit
```
file .\build\debug\app.exe  
set args one two --flag=42  
set environment LOG_LEVEL=debug  
set environment BOOST_ASIO_DISABLE_THREADS=1  
break main.cpp:21  
run  
print argv[1]  
print argv[2]  
print argv[3]  
print slog_level  
print s_threads
```

```
gdb -x .gdbinit
```

### .lldbinit
```
target create .\build\debug\app.exe  
settings set target.run-args one two flag=42  
settings set target.env-vars LOG_LEVEL=debug BOOST_ASIO_DISABLE_THREADS=1  
breakpoint set --file amin.cpp --line 21  
run  
print argv[1]  
print argv[2]  
print argv[3]  
print slog_level  
print s_threads
```

```
lldb -s .lldbinit
```
