---
tags:
  - programming-language
  - cmake
---
[[programming languages/cmake/_|<=]]

`target_include_directories`, `target_compile_definitions`, `target_compile_options` - три команды для настройки свойств цели: пути к заголовкам, макроопределения и флаги компиляции. Все они работают с ключевыми словами PUBLIC / PRIVATE / INTERFACE, которые управляют распространением настройки на зависимые цели.

**`target_include_directories`**

Задаёт каталоги поиска заголовочных файлов для цели.

```cmake
target_include_directories(mylib
    PUBLIC  include          # видят и mylib, и её потребители
    PRIVATE src              # видит только mylib при своей сборке
)
```

Частый паттерн с разными путями для сборки и для установки:

```cmake
target_include_directories(mylib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
```

Здесь генераторные выражения подставляют разный путь в зависимости от того, собирается библиотека внутри проекта или используется после установки.

**`target_compile_definitions`**

Добавляет препроцессорные макросы (эквивалент `-D` компилятора).

```cmake
target_compile_definitions(mylib
    PRIVATE BUILDING_MYLIB        # define BUILDING_MYLIB
    PUBLIC  MYLIB_VERSION=2       # define MYLIB_VERSION 2
)
```

Не нужно писать `-D` — только имя макроса. Типичное применение — условная компиляция:

```cmake
target_compile_definitions(myapp PRIVATE
    $<$<CONFIG:Debug>:ENABLE_LOGGING>   # только в Debug
)
```

**`target_compile_options`**

Передаёт произвольные флаги компилятору — то, что не покрывается специализированными командами (предупреждения, оптимизации, расширения).

```cmake
target_compile_options(mylib PRIVATE
    -Wall -Wextra -Wpedantic
)
```

Флаги зависят от компилятора, поэтому их часто оборачивают в генераторные выражения:

```cmake
target_compile_options(mylib PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)
```

**О ключевых словах видимости**

Принцип одинаков для всех трёх команд:

PRIVATE — настройка применяется только к самой цели, потребители её не наследуют. Подходит для внутренних деталей реализации (приватные исходники, внутренние флаги).

INTERFACE — настройка применяется только к потребителям, но не к самой цели. Используется для header-only библиотек, где у самой цели нет шагов компиляции.

PUBLIC — настройка применяется и к цели, и к потребителям (это PRIVATE + INTERFACE). Подходит для публичных заголовков и макросов из API.

**Сравнительный пример**

```cmake
add_library(mylib STATIC src/core.cpp)

target_include_directories(mylib
    PUBLIC  include       # API-заголовки — нужны всем
    PRIVATE src           # внутренние — только при сборке mylib
)

target_compile_definitions(mylib
    PUBLIC  MYLIB_ENABLED     # макрос виден в API
    PRIVATE MYLIB_INTERNAL    # только внутри реализации
)

target_compile_options(mylib
    PRIVATE -Wall -Wextra     # строгие предупреждения только для своего кода
)
```

При линковке `target_link_libraries(myapp PRIVATE mylib)` цель `myapp` автоматически унаследует путь `include` и макрос `MYLIB_ENABLED`, но не получит `src`, `MYLIB_INTERNAL` и флаги предупреждений — они помечены как PRIVATE.

**Главный принцип**

Решая, какое ключевое слово выбрать, задайте два вопроса: нужна ли настройка самой цели при её сборке (если да — PRIVATE или PUBLIC) и нужна ли она тем, кто использует цель (если да — INTERFACE или PUBLIC). Это и есть суть современного «target-based» CMake — каждая цель сама описывает свои требования и автоматически передаёт нужное потребителям.

## Пример 1: `target_include_directories`

Библиотека с разделением публичных и приватных заголовков.

```
target_include_directories/
├── CMakeLists.txt
├── include/
│   └── geometry/
│       └── circle.hpp      ← публичный API
├── src/
│   ├── circle.cpp
│   └── internal_math.hpp   ← приватный заголовок
└── main.cpp
```

### include/geometry/circle.hpp
```cpp
#pragma once  
  
double circle_area(double);
```

### src/internal_math.hpp
```cpp
#pragma once  
  
constexpr double PI = 3.14159265358979323846;
```

### src/circle.cpp
```cpp
#include "geometry/circle.hpp"  
#include "internal_math.hpp"  
  
double circle_area(const double radius) { return PI * radius * radius; }
```

### main.cpp
```cpp
/*  
cmake -B .build  
cmake --build .build  
*/  
#include "geometry/circle.hpp"  
  
#include <iostream>  
#include <format>  
  
#include "include/geometry/circle.hpp"  
  
int main() {  
    constexpr double RADIUS{2.0};  
    std::cout << std::format("circle_area({}) = {}", RADIUS, circle_area(RADIUS));  
}
```

```
circle_area(2) = 12.566370614359172
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 4.0.0)  
project(Geometry LANGUAGES CXX)  
  
set(CMAKE_CXX_STANDARD 26)  
  
add_library(geometry STATIC src/circle.cpp)  
  
target_include_directories(  
        geometry  
        PUBLIC include # потребители видят geometry/circle.hpp  
        PRIVATE src # internal_math.hpp доступен только при сборке geometry  
)  
  
add_executable(app main.cpp)  
target_link_libraries(app PRIVATE geometry)
```


`app` получит путь `include` автоматически, но не `src` — попытка подключить `internal_math.hpp` из `main.cpp` вызовет ошибку компиляции.

## Пример 2: `target_compile_definitions`

Управление функциональностью через макросы, в том числе зависящие от конфигурации.

### main.cpp
```cpp
/*  
cmake -B .build-dbg  
cmake --build .build-dbg --config Debug  
./.build-dbg/app  
  
cmake -B .build-rel  
cmake --build .build-rel --config Release  
./.build-rel/app  
*/  
  
#include <iostream>  
#include <format>  
  
int main() {  
#ifdef ENABLE_LOGGING  
    std::cout << "[LOG] Start\n";  
#endif  
    std::cout << std::format("Version: {}\n", APP_VERSION);  
}
```

### debug
```
[LOG] Start
Version: 42
```

### release
```
Version: 42
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 4.0.0)  
project(App LANGUAGES CXX)  
  
set(CMAKE_CXX_STANDARD 26)  
  
add_executable(app main.cpp)  
  
target_compile_definitions(  
        app  
        PRIVATE  
        APP_VERSION=42 # обычный макрос  
        $<$<CONFIG:Debug>:ENABLE_LOGGING> # только в Debug-сборке  
)
```

## Пример 3: `target_compile_options`

Кроссплатформенные флаги предупреждений.

```cpp
/*  
cmake -B .build  
cmake --build .build  
*/  
#include <iostream>  
  
int main() {  
    std::cout << 42 << '\n';  
}
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 4.0.0)  
project(App LANGUAGES CXX)  
  
set(CMAKE_CXX_STANDARD 26)  
  
add_executable(app main.cpp)  
  
target_compile_options(app PRIVATE  
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-Wpedantic>  
        $<$<CXX_COMPILER_ID:MSVC>:/W4>)
```

При сборке GCC/Clang применят `-Wall -Wextra -Wpedantic`, MSVC — `/W4`. Один и тот же `CMakeLists.txt` работает на всех платформах.
