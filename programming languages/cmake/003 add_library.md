---
tags:
  - programming-language
  - cmake
---
[[programming languages/cmake/_|<=]]

`add_library` создаёт цель-библиотеку, а `target_link_libraries` связывает с ней другие цели. Это основа модульной организации проекта в CMake.

**`add_library`**

```cmake
add_library(mylib STATIC src1.cpp src2.cpp)   # статическая (.a / .lib)
add_library(mylib SHARED src1.cpp src2.cpp)   # динамическая (.so / .dll)
```

- STATIC — код библиотеки вкомпилируется прямо в исполняемый файл. 
- SHARED — библиотека остаётся отдельным файлом, загружается при запуске.

Если тип не указать, используется значение переменной `BUILD_SHARED_LIBS` (по умолчанию STATIC):

```cmake
add_library(mylib src1.cpp src2.cpp)   # тип зависит от BUILD_SHARED_LIBS
```

Отдельно стоит INTERFACE-библиотека — без исходников, только для распространения заголовков и настроек (header-only библиотеки):

```cmake
add_library(mylib INTERFACE)
```

**`target_link_libraries`**

Связывает цель с библиотеками. Ключевые слова PUBLIC / PRIVATE / INTERFACE определяют, как зависимость распространяется дальше:

```cmake
target_link_libraries(myapp PRIVATE mylib)
```

- PRIVATE — библиотека нужна только для сборки самой цели.
- PUBLIC — нужна и цели, и тем, кто линкуется с ней.
- INTERFACE — нужна только потребителям, но не самой цели.

```
target_include_directories(math PUBLIC math)
```

Благодаря `target_include_directories(math PUBLIC math)` цель `myapp` автоматически получает путь к заголовкам `math` — не нужно прописывать его вручную. В этом и состоит сила target-based подхода: цель несёт в себе всё необходимое для своего использования, и потребителю достаточно одной строки `target_link_libraries`.
### Пример

Структура проекта:
```
myproject/
├── CMakeLists.txt
├── app/
│   └── main.cpp
└── math/
    ├── math.hpp
    └── math.cpp
```

### math/math.hpp
```cpp
#pragma once  
  
int add(int, int);
```

### math/math.cpp
```cpp
#include "math.hpp"  
  
int add(const int a, const int b) { return a + b; }
```

### app/main.cpp
```cpp
/*  
cmake -B .build  
cmake --build .build  
*/  
  
#include "math.hpp"  
  
#include <iostream>  
#include <format>  
  
int main() {  
    const int A{42};  
    const int B{43};  
    std::cout << std::format("add({}, {}) = {}", A, B, add(A, B));  
}
```

```
add(42, 43) = 85
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 4.0.0)  
project(DemoProject LANGUAGES CXX)  
  
set(CMAKE_CXX_STANDARD 26)  
  
# Библиотека  
add_library(math STATIC math/math.cpp)  
  
# Заголовки math доступны всем, кто линкуется с библиотекой  
target_include_directories(math PUBLIC math)  
  
# Исполняемый файл  
add_executable(myapp app/main.cpp)  
  
# Связываем приложение с библиотекой  
target_link_libraries(myapp PRIVATE math)
```
