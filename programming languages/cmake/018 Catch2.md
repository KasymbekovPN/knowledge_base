---
tags:
  - programming-language
  - cmake
---
[[programming languages/cmake/_|<=]]
## Catch2

## Зачем фреймворк поверх голого CTest

Напомню ограничение ручных тестов: `assert` обрывает программу на первом провале, нет читаемых сообщений, нет группировки. Фреймворк решает это — даёт макросы проверок, которые продолжают выполнение после провала, печатают ожидаемое и фактическое значения, организуют тесты в наборы. CTest при этом остаётся раннером верхнего уровня.

Более лёгкий и современный фреймворк с минималистичным синтаксисом. Тоже подключается через `FetchContent` и интегрируется с CTest.

### Подключение и полный пример

Структура:

```
project/
├── CMakeLists.txt
├── include/calc/calc.hpp
├── src/calc.cpp
└── tests/calc_test.cpp
```

### include/calc/calc.hpp
```cpp
#pragma once  
  
int add(const int, const int);  
int divide(const int, const int);
```

### src/calc.cpp
```cpp
#include "calc/calc.hpp"  
#include <stdexcept>  
  
int add(const int a, const int b) { return a + b; }  
  
int divide(const int a, const int b) {  
    if (b == 0) throw std::invalid_argument("Division by zero");  
    return a / b;  
}
```

### tests/calc_test.cpp
```cpp
/*  
cmake -B .build  
cmake --build .build  
ctest --test-dir .build --output-on-failure  
*/  
  
#include <catch2/catch_test_macros.hpp>  
#include "calc/calc.hpp"  
  
TEST_CASE("Add", "[calc]") {  
    REQUIRE(add(2, 3) == 5);  
    REQUIRE(add(-1, 1) == 0);  
}  
  
TEST_CASE("Divide", "[calc]") {  
    REQUIRE(divide(10, 2) == 5);  
  
    SECTION("divide by zeto throws exception") {  
        REQUIRE_THROWS_AS(divide(1, 0), std::invalid_argument);  
    }
}
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.16)  
project(CalcProject LANGUAGES CXX)  
  
enable_testing()  
  
add_library(calc STATIC src/calc.cpp)  
target_include_directories(calc PUBLIC include)  
target_compile_features(calc PUBLIC cxx_std_20)  
  
# --- Загрузка Catch2 ---  
include(FetchContent)  
FetchContent_Declare(  
        Catch2  
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git        GIT_TAG        v3.5.2)  
FetchContent_MakeAvailable(Catch2)  
  
# --- Тестовый исполняемый файл ---  
add_executable(calc_test tests/calc_test.cpp)  
target_link_libraries(calc_test PRIVATE calc Catch2::Catch2WithMain)  
  
# --- Автоматическая регистрация тестов в CTest ---  
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)  
include(Catch)  
catch_discover_tests(calc_test)
```

```
Test project C:/projects/knowledge_base/programming languages/cmake/code/018 Catch2/.build
    Start 1: Add
1/2 Test #1: Add ..............................   Passed    0.01 sec
    Start 2: Divide
2/2 Test #2: Divide ...........................   Passed    0.01 sec

100% tests passed, 0 tests failed out of 2

Total Test time (real) =   0.03 sec
```

### Ключевые моменты Catch2-интеграции

`Catch2::Catch2WithMain` — аналог `gtest_main`: предоставляет `main()`. Для своего `main` используют `Catch2::Catch2`.

`catch_discover_tests(calc_test)` — полный аналог `gtest_discover_tests`: запускает бинарник, находит все `TEST_CASE` и регистрирует их в CTest по отдельности.

`list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)` — нужная строка: модуль `Catch.cmake` (с командой `catch_discover_tests`) лежит в каталоге `extras` внутри скачанных исходников Catch2, и этот путь надо добавить, чтобы `include(Catch)` его нашёл. Переменная `catch2_SOURCE_DIR` появляется автоматически после `FetchContent_MakeAvailable(Catch2)`.

### Особенность Catch2: SECTION

Обрати внимание на `SECTION` в примере. Это отличительная черта Catch2: секции внутри `TEST_CASE` позволяют разделить общую подготовку и разные ветки проверки, при этом код до секции выполняется заново для каждой секции. Удобно для тестирования одного объекта в разных состояниях без дублирования setup-кода. У GoogleTest аналогичную роль играют фикстуры (`TEST_F`).
