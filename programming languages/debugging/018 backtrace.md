---
tags:
  - programming-language
  - debug
  - gdb
  - lldb
---
[[programming languages/debugging/_|<=]]

# Backtrace и навигация по фреймам

## Зачем `bt full`, если есть `bt`

- **`bt`** (`backtrace`) — список фреймов: имя функции, файл, строка, аргументы
- **`bt full`** — то же самое, **плюс все локальные переменные каждого фрейма** — не нужно переключаться в каждый фрейм и делать `info locals` вручную

## Навигация

- **`frame N`** (`f N`) — переключиться на конкретный фрейм по номеру (0 = текущий, самый глубокий)
- **`up`** — подняться на уровень выше (к вызывающей функции)
- **`down`** — спуститься на уровень ниже (к вызванной функции)

Переключение фрейма **не меняет** выполнение программы — просто меняет "контекст", в котором `print`/`info locals` ищут переменные.

## Сводная таблица

| Действие                                 | GDB               | LLDB                                                           |
| ---------------------------------------- | ----------------- | -------------------------------------------------------------- |
| Backtrace (только сигнатуры)             | `bt`              | `bt` / `thread backtrace`                                      |
| Backtrace + все локальные каждого фрейма | `bt full`         | нет единой команды — обход `frame select N` + `frame variable` |
| Ограничить глубину                       | `bt 3`            | `bt 3`                                                         |
| Перейти к фрейму N                       | `frame N` / `f N` | `frame select N` / `f N`                                       |
| Вверх (к вызывающему)                    | `up`              | `up`                                                           |
| Вниз (к вызванному)                      | `down`            | `down`                                                         |
| По умолчанию доходит до                  | `main`            | `_start` / libc-обвязки                                        |

## Полезный нюанс для навигации в GDB

```
(gdb) up 2      # подняться сразу на 2 уровня
(gdb) down 2    # спуститься сразу на 2 уровня
```

LLDB поддерживает то же самое: `up 2`, `down 2`.

## Пример

### main.cpp
```cpp
#include <iostream>  
#include <format>  
#include <string>  
  
int level_three(const int x) {  
    const int squared{x * x};  
    const int doubled{2 * x};  
  
    return squared + doubled; // <-- breakpoint здесь, самый глубокий фрейм  
}  
  
int level_two(const int a, const int b) {  
    std::string label{"processing level_two"};  
    const int sum{a + b};  
    const int result{level_three(sum)};  
  
    return 2 * result;  
}  
  
int level_one(const int input) {  
    std::string context{"top-level call"};  
    const int adjusted{10 + input};  
    const int final_result{level_two(adjusted, input)};  
  
    return final_result;  
}  
  
int main() {  
    constexpr int value{5};  
    const int answer{level_one(value)};  
  
    std::cout << std::format("answer: {}\n", answer);  
  
    return 0;  
}  
  
/*  
  
###  
lldb .\build\debug\app.exe  
breakpoint set --file main.cpp --line 9  
run  
bt  
up  
frame variable  
frame select 3  
  
###  
C:\msys64\clang64\bin\gdb.exe .\build\debug\app.exe  
break main.cpp:9  
run  
bt full  
up  
print a  
frame 3  
  
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
