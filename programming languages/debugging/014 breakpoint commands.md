---
tags:
  - programming-language
  - debug
  - gdb
  - lldb
---
[[programming languages/debugging/_|<=]]

# breakpoint commands

Самая недооценённая фича: можно привязать **последовательность команд**, которая выполнится **автоматически** при каждом попадании в breakpoint — без твоего участия. Незаменимо для логирования состояния на лету без ручного набора команд каждый раз.

**Обязательно `silent` в `commands`**, если не хочешь захламления вывода — иначе каждая остановка печатает служебный "Breakpoint N, function() at file:line" даже при полностью автоматическом прогоне

### GDB

```
(gdb) break main.cpp:26
(gdb) commands 1
> print i
> print argc
> continue
> end
```

Синтаксис: `commands <breakpoint_number>`, затем список команд построчно, завершается `end`. Последняя команда — часто `continue`, чтобы breakpoint не блокировал выполнение, а просто "логировал на лету".

### LLDB

```
(lldb) breakpoint set --file main.cpp --line 26
(lldb) breakpoint command add 1
> print i
> print argc
> continue
> DONE
```

Завершается словом `DONE` на отдельной строке (или `Ctrl+D`).

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
breakpoint set --file main.cpp --line 5  
breakpoint command add 1  
> print i  
> continue  
> DONE  
  
###  
C:\msys64\clang64\bin\gdb.exe .\build\debug\app.exe  
break main.cpp:5  
commands 1  
> print i  
> continue  
> end  
  
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
