---
tags:
  - programming-language
  - debug
  - gdb
  - lldb
---
[[programming languages/debugging/_|<=]]

## Сравнение синтаксиса и формата вывода

| Действие                       | GDB                                      | LLDB                                                        |
| ------------------------------ | ---------------------------------------- | ----------------------------------------------------------- |
| Вся структура                  | `print emp` → `{name = ..., age = ...}`  | `print emp` → `(Employee) {\n name = ...\n}` — многострочно |
| Массив                         | `print numbers` → `{10, 20, 30, 40, 50}` | `print numbers` → `([0] = 10, [1] = 20, ...)` — с индексами |
| Разыменование                  | `print *ptr`                             | `print *ptr` — идентично                                    |
| Доступ через указатель         | `emp_ptr->name`                          | `emp_ptr->name` — идентично                                 |
| Срез памяти произвольной длины | `print *ptr@3` (оператор `@`)            | **нет прямого аналога** — обходной путь ниже                |

## LLDB — как посмотреть срез памяти без `@`

| Что нужно                     | GDB                   | LLDB                  |
| ----------------------------- | --------------------- | --------------------- |
| Структура целиком             | `print emp`           | `print emp`           |
| Поле структуры                | `print emp.age`       | `print emp.age`       |
| Вложенное поле                | `print emp.office.x`  | `print emp.office.x`  |
| Через указатель               | `print emp_ptr->name` | `print emp_ptr->name` |
| Массив целиком                | `print numbers`       | `print numbers`       |
| Элемент массива               | `print numbers[2]`    | `print numbers[2]`    |
| Разыменование указателя       | `print *ptr`          | `print *ptr`          |
| Срез N элементов из указателя | `print *ptr@N`        | `print *(T(*)[N])ptr` |
| Только адрес                  | `print &emp`          | `print &emp`          |

## Пример

### main.cpp
```cpp
#include <iostream>  
#include <string>  
  
struct Point {  
    int x;  
    int y;  
};  
  
struct Employee {  
    std::string name;  
    int age;  
    Point office;      // вложенная структура  
    double salary[3];  // массив внутри структуры (бонусы по кварталам)  
};  
  
int main() {  
    int count = 42;                 // <-- breakpoint здесь  
    unsigned char byte_val = 0xA5;   // 165 в десятичном, удобно для /x /t /o  
    char letter = 'Z';  
    int numbers[5] = {10, 20, 30, 40, 50};  
    int* ptr = &numbers[2];  
  
    Point p1{10, 20};  
    Employee emp{"Pablo", 35, Point{1, 4}, {1000.0, 1500.0, 1200.0}};  
    Employee* emp_ptr = &emp;  
  
    for (int i = 0; i < 5; ++i) {  
        count += numbers[i];        // <-- сюда попробуем display count / display/x count  
        std::cout << "count = " << count << "\n";  
    }  
    std::cout << "byte_val = " << static_cast<int>(byte_val) << "\n";  
    std::cout << "letter = " << letter << "\n";  
    std::cout << "*ptr = " << *ptr << "\n";  
    std::cout << "emp.name = " << emp.name << ", office=(" << emp.office.x << "," << emp.office.y << ")\n";  // <-- breakpoint сюда  
    return 0;  
}  
  
/*  
  
###  
lldb .\build\debug\app.exe  
breakpoint set --file main.cpp --line 29  
run  
print emp.age  
print emp.office.x  
print emp_ptr->name  
print numbers  
print numbers[2]  
print ptr  
print *ptr  
  
###  
C:\msys64\clang64\bin\gdb.exe .\build\debug\app.exe  
break main.cpp:29  
run  
print emp.age  
print emp.office.x  
print emp_ptr->name  
print numbers  
print numbers[2]  
print ptr  
print *ptr  
  
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
