---
tags:
  - programming-language
  - debug
  - gdb
  - lldb
---
[[programming languages/debugging/_|<=]]

# Disable/enable breakpoints

Полезно, когда breakpoint ещё понадобится, но сейчас мешает (например, внутри частого цикла).

### GDB

```
(gdb) disable 2          # выключить конкретный breakpoint по номеру
(gdb) enable 2           # включить обратно
(gdb) disable             # выключить ВСЕ breakpoints
(gdb) info breakpoints    # посмотреть какие enabled/disabled
```

### LLDB

```
(lldb) breakpoint disable 2
(lldb) breakpoint enable 2
(lldb) breakpoint disable    # выключить все
(lldb) breakpoint list
```

## Пример

### main.cpp
```cpp
#include <iostream>  
  
int main(int argc, char *argv[]) {  
    for (int i{7}; i < 10; ++i) {  
        std::cout << i << std::endl;  
        std::cout << i*i << std::endl;  
        std::cout << i*i*i << std::endl;  
    }  
    return 0;  
}  
  
/*  
  
###  
lldb .\build\debug\app.exe  
breakpoint set --file main.cpp --line 5  
breakpoint set --file main.cpp --line 6  
breakpoint set --file main.cpp --line 7  
breakpoint list  
breakpoint disable 1  
breakpoint disable 3  
  
###  
C:\msys64\clang64\bin\gdb.exe .\build\debug\app.exe  
break main.cpp:5  
break main.cpp:6  
break main.cpp:7  
info breakpoints  
disable 1  
disable 3  
  
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
