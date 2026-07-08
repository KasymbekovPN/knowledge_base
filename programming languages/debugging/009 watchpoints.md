---
tags:
  - programming-language
  - debug
  - gdb
  - lldb
---
[[programming languages/debugging/_|<=]]

# Watchpoints — отслеживание изменения переменных/памяти

## Что это и зачем

Breakpoint останавливает выполнение **в конкретном месте кода**. Watchpoint останавливает выполнение **при изменении значения** конкретной переменной/памяти — независимо от того, в какой строке или функции это произошло. Незаменимо для багов вида "кто-то посреди программы неожиданно меняет эту переменную, а я не знаю, где".

## Три вида watchpoint'ов

|Тип|GDB|LLDB|Срабатывает при|
|---|---|---|---|
|Write|`watch`|`watchpoint set variable` (по умолчанию)|Записи в память|
|Read|`rwatch`|`--watch read`|Чтении из памяти|
|Read/Write|`awatch`|`--watch read_write`|Любом доступе|

---

## GDB — синтаксис

```
(gdb) watch a                     # следить за переменной a (пока в скоупе)
(gdb) watch *(int*)0x7fffffffe950 # следить по конкретному адресу
(gdb) rwatch counter               # остановка при ЧТЕНИИ
(gdb) awatch counter               # остановка при чтении ИЛИ записи
```

Список активных watchpoint'ов:

```
(gdb) info watchpoints
```

## LLDB — синтаксис

```
(lldb) watchpoint set variable a
(lldb) watchpoint set expression -- &a       # по адресу через выражение
(lldb) watchpoint set variable counter --watch read
(lldb) watchpoint set variable counter --watch read_write
```

Список:

```
(lldb) watchpoint list
```

## Аппаратные vs программные watchpoints

- **Hardware watchpoint** — использует debug-регистры процессора (`DR0-DR3` на x86), проверка происходит **на уровне CPU**, почти бесплатно по производительности. И GDB, и LLDB используют это по умолчанию, если возможно.
- **Software watchpoint** — если аппаратных слотов не хватает (обычно **4** на x86-64) или архитектура не поддерживает — отладчик **вручную** сверяет значение после **каждой** машинной инструкции (`singlestep`). Это на порядки медленнее (может замедлить программу в 100+ раз).

Проверить, что используется:

```
(gdb) info watchpoints    # покажет "hw watchpoint" или "watchpoint"
```

## Watchpoint по адресу памяти (не только по имени переменной)

Полезно, когда переменная выходит из скоупа, но память ещё жива (например, поле в куче):

**GDB:**

```
(gdb) print &a
$1 = (long long *) 0x7fffffffe8e0
(gdb) watch *(long long*)0x7fffffffe8e0
```

**LLDB:**

```
(lldb) watchpoint set expression -- &a
```

## Условные watchpoints (комбинация с условиями, как в breakpoints)

**GDB:**

```
(gdb) watch a if a > 100
```

**LLDB:**

```
(lldb) watchpoint set variable a
(lldb) watchpoint modify --condition "a > 100" 1
```

##  Пример

### main.cpp
```cpp
#include <iostream>  
#include <format>  
  
int main(int argc, char *argv[]) {  
  
    int a{};  
    a = 42;  
    std::cout << a << std::endl;  
  
    return 0;  
}  
  
/*  
  
lldb .\build\debug\app.exe  
breakpoint set --name main  
run  
watchpoint set variable a --watch read_write  
continue  
  
C:\msys64\clang64\bin\gdb.exe .\build\debug\app.exe  
break main  
run  
awatch a  
continue  
  
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
