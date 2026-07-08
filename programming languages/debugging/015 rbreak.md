---
tags:
  - programming-language
  - debug
  - gdb
  - lldb
---
[[programming languages/debugging/_|<=]]

Ставит breakpoint **сразу на все функции**, чьи имена подходят под regex. Незаменимо для большого монолитного кодабейса — например, поймать все методы конкретного класса разом.

### GDB

```
(gdb) rbreak ^Generator<.*>::.*    # все методы шаблона Generator<T>
(gdb) rbreak ^validate$             # конкретная функция (но через regex-движок)
```

 GDB использует **POSIX Basic Regular Expressions (BRE)**, где `|` для alternation нужно **экранировать** как `\|` (в отличие от привычного PCRE/extended regex, где `|` работает "как есть"). Это неочевидная, но важная деталь:

```
(gdb) rbreak next\|value    ← правильно для GDB (BRE)
(gdb) rbreak next|value      ← НЕ сработает
```

### LLDB — тот же смысл, другая команда

```
(lldb) breakpoint set --func-regex "next|value"
```

## Важные грабли, которые стоит запомнить

1. **GDB regex — BRE, не PCRE** — `\|`, `\+`, `\?` нужно экранировать для спецсмысла (в отличие от большинства современных regex-движков)
2. **`rbreak` матчит против имени функции, но включает namespace/шаблонные параметры** — оттого `"value"` неожиданно поймал и `yield_value`, и `value()` разом — если regex слишком широкий, легко "выстрелить себе в ногу" и наставить breakpoint'ов в разы больше, чем ожидалось. Всегда проверяй `info breakpoints`/`breakpoint list` после `rbreak`, прежде чем `continue`

## Пример

### main.cpp
```cpp
#include <iostream>  
#include <format>  
  
template<typename T>  
struct Value {  
    T value;  
  
    explicit Value(T _value) noexcept : value(_value) {}  
  
    T get() const noexcept {  
        return value;  
    }};  
  
void test_int(const int x) {  
    std::cout << std::format("test_int: {}\n", x);  
}  
  
void test_double(const double x) {  
    std::cout << std::format("test_double: {}\n", x);  
}  
  
void print_str(const std::string s) {  
    std::cout << std::format("print_str: {}\n", s);  
}  
  
int main(int argc, char *argv[]) {  
    test_int(1234);  
    test_double(12.34);  
    print_str("hello world");  
  
    const Value<int> v{42};  
    std::cout << v.get() << '\n';  
  
    return 0;  
}  
  
/*  
  
###  
lldb .\build\debug\app.exe  
breakpoint set --func-regex "test|print"  
breakpoint set --func-regex "Value<.*>::.*"  
  
###  
C:\msys64\clang64\bin\gdb.exe .\build\debug\app.exe  
rbreak test\|print  
rbreak ^Value::<.*>  
  
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
