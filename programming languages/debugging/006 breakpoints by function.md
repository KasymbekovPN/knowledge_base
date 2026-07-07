---
tags:
  - programming-language
  - debug
  - gdb
  - lldb
---
[[programming languages/debuging/_|<=]]

# Breakpoints: по функции, по файлу:строке, условные

## 1. Breakpoint по функции

### GDB

```
(gdb) break main
(gdb) break fibonacci
(gdb) break Generator<long long>::next
```

Для шаблонных/перегруженных функций GDB иногда просит уточнить — если есть несколько инстанциаций, покажет список на выбор:

```
(gdb) break next
Breakpoint 1 (2 locations) 
```

### LLDB

```
(lldb) breakpoint set --name main
(lldb) b fibonacci
(lldb) breakpoint set --name "Generator<long long>::next"
```

LLDB для шаблонов часто требует **явного экранирования кавычками**, если в имени есть `<>` или `::` с шаблонными параметрами — иначе может не распарсить.

**Практический нюанс:** для функций из библиотек (Boost.Asio, STL) breakpoint по имени часто ловит **несколько** мест сразу (все инстанциации шаблона) — полезно, если хочешь поймать вызов из любой точки кода, но может дать много лишних остановок.

### Пример

### main.cpp
```cpp
#include <iostream>  
#include <format>  
  
template <typename T>  
struct Value {  
    T value;  
  
    explicit Value(T _value) noexcept : value{_value} {}  
    T getValue() const noexcept { return value; }  
};  
  
void test_print() {  
    std::cout << "test_print\n";  
}  
  
int main(int argc, char *argv[]) {  
    constexpr int value{123};  
    std::cout << std::format("value: {}\n", value);  
  
    test_print();  
  
    const Value<double> v{12.34};  
    std::cout << std::format("v.getValue(): {}\n", v.getValue());  
  
    return 0;  
}  
  
/*  
  
lldb .\build\debug\app.exe  
breakpoint set --name main  
b test_print  
breakpoint set --name "Value<double>::getValue"  
run  
  
C:\msys64\clang64\bin\gdb.exe .\build\debug\app.exe  
break main  
break test_print  
break Value<double>::getValue  
run  
  
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
