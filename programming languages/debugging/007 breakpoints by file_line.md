---
tags:
  - programming-language
  - debug
  - gdb
  - lldb
---
[[programming languages/debuging/_|<=]]

## 2. Breakpoint по файлу:строке

### GDB

```
(gdb) break main.cpp:26
(gdb) break coro_demo.cpp:45
```

Если файл не найден по короткому имени (несколько файлов с одинаковым именем в проекте — актуально для тебя с большой монолитной кодовой базой), указывай полный путь:

```
(gdb) break /home/user/project/src/main.cpp:26
```

### LLDB

```
(lldb) breakpoint set --file main.cpp --line 26
(lldb) b main.cpp:26
```

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
    
void test_print() { std::cout << "test_print\n"; }    
    
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
breakpoint set --file main.cpp --line 18  
b C:\projects\knowledge_base\programming languages\debuging\code\007 breakpoints by file_line\main.cpp:23  
run  
  C:\msys64\clang64\bin\gdb.exe .\build\debug\app.exe  
break main.cpp:18  
break C:\projects\knowledge_base\programming languages\debuging\code\007 breakpoints by file_line\main.cpp:23  
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
