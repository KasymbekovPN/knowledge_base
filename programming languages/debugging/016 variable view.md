---
tags:
  - programming-language
  - debug
  - gdb
  - lldb
---
[[programming languages/debugging/_|<=]]

# Просмотр переменных: print, display, форматированный вывод

## `print` vs `display` — в чём разница

- **`print`** (`p`) — показывает значение **один раз**, по запросу
- **`display`** — добавляет выражение в список "автопоказа": оно печатается **автоматически после каждой остановки** (после `step`/`next`/breakpoint), без повторного набора команды

## Форматы вывода — общий синтаксис

```
print/<формат> <выражение>
```

|Формат|Означает|Пример вывода для `42`|
|---|---|---|
|`/x`|hex (шестнадцатеричный)|`0x2a`|
|`/t`|binary (двоичный)|`101010`|
|`/o`|octal (восьмеричный)|`052`|
|`/d`|signed decimal|`42`|
|`/u`|unsigned decimal|`42`|
|`/c`|как символ (char)|`42 '*'`|
|`/f`|как float|`42` (интерпретирует биты как float)|
|`/a`|как адрес (+ символ, если есть)|`0x2a <symbol+N>`|
|`/s`|как C-строку|—|

## Сводная таблица

|Действие|GDB|LLDB|
|---|---|---|
|Разовый просмотр|`print var`|`print var` (или родное `expression var`)|
|Hex|`print/x var`|`print/x var` (алиас) или `expression -f x -- var`|
|Binary|`print/t var`|`expression -f binary -- var`|
|Octal|`print/o var`|`expression -f octal -- var`|
|Как символ|`print/c var`|`expression -f char -- var`|
|Как адрес|`print/a var`|`expression -f address -- var`|
|Автопоказ на каждой остановке|`display var`|`target stop-hook add -o 'frame variable var'`|
|Список автопоказов|`info display`|`target stop-hook list`|
|Убрать автопоказ|`undisplay N`|`target stop-hook delete N`|

## Практическое наблюдение

`display` в GDB привязан к **конкретному breakpoint's контексту** (переменная должна существовать в текущем скоупе на момент остановки — иначе просто молча пропускается), а LLDB `stop-hook` — более "тяжёлый", глобальный механизм, который срабатывает на **любой** остановке потока в программе, не только на конкретном breakpoint. Если тебе нужна привязка именно к конкретной точке — комбинируй `target stop-hook add --file main.cpp --line 26 -o '...'`, чтобы ограничить область действия.

## Пример

### main.cpp
```cpp
#include <iostream>  
#include <format>  
  
int main() {  
    int count{42}; // <-- breakpoint здесь  
    unsigned char byte_val{0xA5}; // 165 в десятичном, удобно для /x /t /o  
    char letter = 'Z';  
    int numbers[5] = {10, 20, 30, 40, 50};  
    int* ptr = &numbers[2];  
  
    for (int i{}; i < 5; ++i) {  
        count += numbers[i]; // <-- сюда попробуем display count / display/x count  
        std::cout << std::format("count: {}\n", count);  
    }  
    std::cout << std::format("byte_val: {}\n", static_cast<int>(byte_val));  
    std::cout << std::format("letter: {}\n", letter);  
    std::cout << std::format("*ptr: {}\n", *ptr);  
  
    return 0;  
}  
  
/*  
  
###  
lldb .\build\debug\app.exe  
breakpoint set --file main.cpp --line 12  
run  
target stop-hook add -o 'frame variable count'  
print/x byte_val  
expression -f binary -- byte_val  
continue  
  
###  
C:\msys64\clang64\bin\gdb.exe .\build\debug\app.exe  
break main.cpp:12  
run  
display count  
display/x count  
print byte_val  
print/x byte_val  
print/t byte_val  
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
