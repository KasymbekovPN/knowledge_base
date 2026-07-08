---
tags:
  - programming-language
  - debug
  - gdb
  - lldb
---
[[programming languages/debugging/_|<=]]

# Temporary breakpoints

Срабатывает **один раз**, затем автоматически удаляется. Удобно для "долети до этой точки один раз, дальше не мешай".

### GDB

```
(gdb) tbreak main.cpp:26
```

### LLDB

```
(lldb) breakpoint set --file main.cpp --line 26 --one-shot true
(lldb) b main.cpp:26 -o        # короткий флаг тоже работает
```

## Пример

### main.cpp
```cpp
#include <iostream>  
  
int main(int argc, char *argv[]) {  
    for (int i{7}; i < 10; ++i) {  
        std::cout << i << std::endl;  
    }  
    return 0;  
}  
  
/*  
  
###  
lldb .\build\debug\app.exe  
breakpoint set --file main.cpp --line 5 --one-shot true  
  
###  
C:\msys64\clang64\bin\gdb.exe .\build\debug\app.exe  
tbreak main.cpp:5  
  
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
