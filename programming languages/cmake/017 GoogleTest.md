---
tags:
  - programming-language
  - cmake
---
[[programming languages/cmake/_|<=]]

## Зачем фреймворк поверх голого CTest

Напомню ограничение ручных тестов: `assert` обрывает программу на первом провале, нет читаемых сообщений, нет группировки. Фреймворк решает это — даёт макросы проверок, которые продолжают выполнение после провала, печатают ожидаемое и фактическое значения, организуют тесты в наборы. CTest при этом остаётся раннером верхнего уровня.

## GoogleTest

Самый распространённый фреймворк в C++. Подключается через `FetchContent`, а его тесты регистрируются в CTest автоматически.

### Ключевые моменты GoogleTest-интеграции

`GTest::gtest_main` — цель, предоставляющая готовую функцию `main()`, поэтому в тестовом файле её писать не нужно. Если нужен свой `main` (для особой инициализации), линкуют `GTest::gtest` вместо `gtest_main`.

`gtest_discover_tests(calc_test)` — самое важное. Эта команда из модуля `GoogleTest` **запускает** собранный тестовый бинарник на этапе сборки, чтобы узнать список всех определённых в нём тестов, и регистрирует **каждый** из них как отдельный тест CTest. Не нужно писать `add_test` вручную — три `TEST(...)` из примера станут тремя отдельными записями в CTest:

```
CalcTest.Add
CalcTest.Divide
CalcTest.DivideByZeroThrows
```

`set(gtest_force_shared_crt ON ...)` — важно на Windows: без этого gtest может собираться со статической C-runtime, конфликтуя с твоим проектом, что даёт ошибки линковки. На Unix не влияет.

### Частые макросы GoogleTest

`EXPECT_*` продолжает выполнение теста после провала, `ASSERT_*` прерывает текущий тест. Основные:

```cpp
EXPECT_EQ(a, b);            // равно
EXPECT_NE(a, b);            // не равно
EXPECT_TRUE(cond);          // истинно
EXPECT_FALSE(cond);         // ложно
EXPECT_LT(a, b);            // меньше (и GT/LE/GE)
EXPECT_NEAR(a, b, eps);     // близко для float
EXPECT_THROW(expr, Type);   // выбросит исключение указанного типа
EXPECT_NO_THROW(expr);      // не выбросит
EXPECT_STREQ(s1, s2);       // равенство C-строк
```

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
  
#include <gtest/gtest.h>  
  
#include "calc/calc.hpp"  
  
TEST(CalcTest, Add) {  
    EXPECT_EQ(add(2, 3), 5);  
    EXPECT_EQ(add(-1, 1), 0);  
}  
  
TEST(CalcTest, Divide) {  
    EXPECT_EQ(divide(10, 2), 5);  
}  
  
TEST(CalcTest, DivideByZeroThrows) {  
    EXPECT_THROW(divide(1, 0), std::invalid_argument);  
}
```

### CMakeTests.txt
```cmake
cmake_minimum_required(VERSION 4.0.0)  
project(CalcProject LANGUAGES CXX)  
  
enable_testing()  
  
add_library(calc STATIC src/calc.cpp)  
target_include_directories(calc PUBLIC include)  
target_compile_features(calc PUBLIC cxx_std_20)  
  
# --- Загрузка GoogleTest ---  
include(FetchContent)  
FetchContent_Declare(  
        googletest  
        GIT_REPOSITORY https://github.com/google/googletest.git        GIT_TAG        v1.14.0)  
  
# На Windows: gtest использует ту же runtime-библиотеку, что и проект  
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)  
FetchContent_MakeAvailable(googletest)  
  
# --- Тестовый исполняемый файл ---  
add_executable(calc_test tests/calc_test.cpp)  
target_link_libraries(calc_test PRIVATE calc GTest::gtest_main)  
  
# --- Автоматическая регистрация тестов в CTest ---  
include(GoogleTest)  
gtest_discover_tests(calc_test)
```

```
Test project C:/projects/knowledge_base/programming languages/cmake/code/017 GoogleTest/.build
    Start 1: CalcTest.Add
1/3 Test #1: CalcTest.Add .....................   Passed    0.01 sec
    Start 2: CalcTest.Divide
2/3 Test #2: CalcTest.Divide ..................   Passed    0.01 sec
    Start 3: CalcTest.DivideByZeroThrows
3/3 Test #3: CalcTest.DivideByZeroThrows ......   Passed    0.01 sec

100% tests passed, 0 tests failed out of 3

Total Test time (real) =   0.04 sec
```
