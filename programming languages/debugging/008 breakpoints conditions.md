---
tags:
  - programming-language
  - debug
  - gdb
  - lldb
---
[[programming languages/debuging/_|<=]]

## 3. Условные breakpoints

### GDB — два способа

**При создании:**

```
(gdb) break main.cpp:26 if i == 5
(gdb) break fibonacci if count > 3
```

**Добавить условие к уже существующему breakpoint:**

```
(gdb) condition 1 i == 5
```

Убрать условие:

```
(gdb) condition 1
```

### LLDB — через флаг

**GDB-style алиас (частично работает для простых случаев, но ненадёжно для сложных условий с пробелами) — лучше сразу использовать явный флаг:**

```
(lldb) breakpoint set --file main.cpp --line 26 --condition "i == 5"
(lldb) b main.cpp:26 -c "i == 5"
```

**Добавить условие к существующему:**

```
(lldb) breakpoint modify --condition "i == 5" 1
```

Убрать:

```
(lldb) breakpoint modify --condition "" 1
```

### Пример

### main.cpp
```cpp
#include <iostream>  
#include <format>  
  
int main(int argc, char *argv[]) {  
  
    for (int i = 1; i < 10; i++) {  
        std::cout << std::format("i: {}\n", i);  
    }  
    for (int i = 1; i < 10; i++) {  
        std::cout << std::format("i: {}\n", i);  
    }  
    return 0;  
}  
  
/*  
  
lldb .\build\debug\app.exe  
breakpoint set --file main.cpp --line 7 --condition "i == 5"  
breakpoint set --file main.cpp --line 11 --condition "i == 7"  
run  
  
C:\msys64\clang64\bin\gdb.exe .\build\debug\app.exe  
break main.cpp:7 if i == 5  
break main.cpp:11 if i == 7  
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
