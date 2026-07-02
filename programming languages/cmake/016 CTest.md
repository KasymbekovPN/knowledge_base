---
tags:
  - programming-language
  - cmake
---
[[programming languages/cmake/_|<=]]

CTest — встроенная в CMake система запуска тестов. Она не тестовый фреймворк (вроде GoogleTest), а **раннер**: запускает исполняемые файлы-тесты, собирает результаты, показывает, что прошло, а что упало. Работает с любым тестом, который сигнализирует об успехе кодом возврата `0`, а о провале — ненулевым.

## Базовый механизм

Три составляющие: включить тестирование, зарегистрировать тесты, запустить их.

**`enable_testing()`** — включает поддержку тестов в проекте. Вызывается один раз, обычно в корневом `CMakeLists.txt`. После этого становится доступна команда `add_test` и генерируется инфраструктура для CTest.

**`add_test`** — регистрирует один тест: даёт ему имя и указывает, какую команду выполнить.

```cmake
add_test(NAME имя_теста COMMAND исполняемый_файл аргументы)
```

**Запуск** — командой `ctest` из каталога сборки.

## Как CTest определяет успех

Ключевой принцип: тест считается **пройденным**, если запущенная команда вернула код `0`, и **проваленным** при любом другом коде возврата. Это соглашение уровня операционной системы, поэтому CTest работает с чем угодно — с бинарником на C++, скриптом на Python, любой программой.

Из этого следует, что простейший тест — это программа, которая возвращает `0` при успехе:

```cpp
// test_math.cpp
#include "math.hpp"
#include <cassert>

int main() {
    assert(square(5) == 25);   // при провале assert вызовет abort() → ненулевой код
    return 0;                   // успех
}
```

## Минимальный полный пример

Структура:

```
project/
├── CMakeLists.txt
├── math.hpp
├── math.cpp
└── test_math.cpp
```

### math.hpp
```cpp
#pragma once  
  
int square(const int);
```

### math.cpp
```cpp
#include "math.hpp"  
  
int square(const int x) { return x * x; }
```

### test_main.cpp
```cpp
/*  
cmake -B .build  
cmake --build .build --config Debug  
ctest --test-dir .build -C Debug  
*/  
  
#include "math.hpp"  
  
#include <cassert>  
  
int main() {  
    assert(square(3) == 9);  
    assert(square(0) == 0);  
    assert(square(-4) == 16);  
  
    return 0;  
}
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 4.0.0)  
project(TestDemo LANGUAGES CXX)  
  
enable_testing()  
  
add_library(math STATIC math.cpp)  
target_include_directories(math PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})  
target_compile_features(math PUBLIC cxx_std_20)  
  
# исполняемый файл теста  
add_executable(test_math test_main.cpp)  
target_link_libraries(test_math PRIVATE math)  
  
# регистрируем его как тест  
add_test(NAME math_test COMMAND test_math)
```

```
Test project C:/projects/knowledge_base/programming languages/cmake/code/016 CTest/minimally/.build
    Start 1: math_test
1/1 Test #1: math_test ........................   Passed    0.02 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.02 sec
```

## Полезные опции ctest

CTest имеет много ключей для управления запуском:

```
ctest                    # запустить все тесты
ctest --output-on-failure   # показать вывод программы для упавших тестов
ctest -V                 # подробный вывод (verbose) для всех тестов
ctest -R math            # запустить только тесты, чьё имя совпадает с regex "math"
ctest -E slow            # исключить тесты, совпадающие с "slow"
ctest -j 8               # запускать тесты параллельно в 8 потоков
ctest --rerun-failed     # перезапустить только упавшие в прошлый раз
ctest -N                 # только перечислить тесты, не запуская
```

`--output-on-failure` — самый полезный на практике: по умолчанию CTest прячет вывод тестов, и при провале не видно, что именно пошло не так. Этот ключ показывает stdout/stderr упавшего теста.

## Свойства тестов

`add_test` регистрирует тест, а `set_tests_properties` настраивает его поведение. Частые сценарии:

**Ожидаемый провал** — тест должен завершиться с ошибкой (проверка, что код корректно отвергает неверный ввод):

```cmake
set_tests_properties(math_test PROPERTIES WILL_FAIL TRUE)
```

**Таймаут** — тест провалится, если выполняется дольше указанного времени:

```cmake
set_tests_properties(math_test PROPERTIES TIMEOUT 10)   # 10 секунд
```

**Проверка вывода по регулярному выражению** — тест пройден, только если в выводе есть/нет определённого текста:

```cmake
set_tests_properties(math_test PROPERTIES
    PASS_REGULAR_EXPRESSION "OK"           # пройден только если в выводе есть "OK"
    FAIL_REGULAR_EXPRESSION "ERROR|FAIL"   # провален, если встретилось это
)
```

**Метки** — группировка тестов для выборочного запуска:

```cmake
set_tests_properties(math_test PROPERTIES LABELS "unit")
```

Затем: `ctest -L unit` запустит только тесты с меткой `unit`.

## Передача аргументов и несколько тестов из одного бинарника

Один исполняемый файл можно зарегистрировать как несколько тестов с разными аргументами:

```cmake
add_executable(calc_test calc_test.cpp)

add_test(NAME test_add      COMMAND calc_test add 2 3 5)
add_test(NAME test_multiply COMMAND calc_test mul 4 5 20)
add_test(NAME test_divide   COMMAND calc_test div 10 2 5)
```

Программа `calc_test` читает аргументы, выполняет соответствующую проверку и возвращает `0`/ненулевой код. CTest покажет три отдельных теста.

## Тесты в многомодульном проекте

В структуре с отдельным каталогом тестов `enable_testing()` вызывается в **корневом** файле (чтобы CTest работал из корня сборки), а `add_test` — в `tests/CMakeLists.txt`:

Корневой `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.16)
project(Project LANGUAGES CXX)

enable_testing()                 # обязательно в корне

add_subdirectory(src)
add_subdirectory(tests)
```

`tests/CMakeLists.txt`:

```cmake
add_executable(core_test core_test.cpp)
target_link_libraries(core_test PRIVATE Project::core)
add_test(NAME core_test COMMAND core_test)
```

Важная деталь: `enable_testing()` должен быть в **корневом** файле, иначе CTest не найдёт тесты при запуске из корня сборки. Если вызвать его только в подкаталоге, тесты зарегистрируются, но не будут видны из корня.

## Запуск из нужного каталога

Начиная с CMake 3.20 удобно запускать тесты, не переходя в каталог сборки:

```
ctest --test-dir build --output-on-failure
```

Раньше приходилось делать `cd build && ctest`.

## Ограничение «ручных» тестов и переход к фреймворкам

Показанный подход с `assert` и кодами возврата работает, но у него есть предел: `assert` при первом же провале обрывает программу, ты не видишь, сколько ещё проверок упало бы, нет читаемых сообщений об ошибках, нет удобной группировки. Для реальных проектов используют тестовый фреймворк (GoogleTest, Catch2, doctest), который даёт выразительные проверки, понятные сообщения и автоматическую регистрацию.

При этом CTest остаётся раннером верхнего уровня: фреймворк предоставляет макросы для написания тестов, а CTest их запускает и агрегирует. GoogleTest интегрируется с CTest через `gtest_discover_tests`, который автоматически регистрирует каждый тест фреймворка как отдельный тест CTest — без ручных `add_test` на каждый случай.

## Практические правила

Вызывай `enable_testing()` один раз в корневом `CMakeLists.txt`, даже если сами тесты объявлены в подкаталогах.

Оборачивай тесты в опцию (`option(PROJECT_BUILD_TESTS ...)`) и отключай их, когда проект подключён как зависимость, — потребителю твоей библиотеки не нужны твои тесты.

Привыкай запускать `ctest --output-on-failure` — без этого ключа причины провала не видны.

Используй свойства тестов (`TIMEOUT`, `LABELS`, `WILL_FAIL`) для управления поведением вместо усложнения самих тестовых программ.

Для чего-то большего, чем пара элементарных проверок, сразу бери тестовый фреймворк — ручные `assert`-тесты быстро упираются в свои ограничения.

---

## GoogleTest vs Catch2: что выбрать

Оба отлично интегрируются с CMake и CTest одинаковым образом (`*_discover_tests`). Разница в характере:

GoogleTest — индустриальный стандарт, богатый набор возможностей (фикстуры, параметризованные тесты, mock-фреймворк GoogleMock в комплекте), больше «церемонии». Выбирают, когда нужны моки или проект большой.

Catch2 — легче, синтаксис лаконичнее, `SECTION` элегантно решает проблему общей подготовки, заголовки проще. Выбирают за минимализм и удобство для небольших и средних проектов.

Для начинающего оба хороши; Catch2 чуть дружелюбнее на старте, GoogleTest даёт больше при росте проекта.

## Общий шаблон интеграции (любой фреймворк)

Независимо от выбора, схема одна:

Загрузить фреймворк через `FetchContent` (самодостаточно, ничего ставить в систему не нужно). Слинковать тестовый бинарник с целью фреймворка, дающей `main`. Подключить модуль обнаружения тестов и вызвать `*_discover_tests` — он зарегистрирует каждый тест в CTest автоматически. Запускать через `ctest --output-on-failure`.

## Оборачивание в опцию

Как обсуждалось в теме тестирования, тесты стоит собирать только когда нужно — и особенно не навязывать их потребителям библиотеки:

```cmake
option(CALC_BUILD_TESTS "Собирать тесты" ${PROJECT_IS_TOP_LEVEL})

if(CALC_BUILD_TESTS)
    enable_testing()
    include(FetchContent)
    # ... загрузка фреймворка и объявление тестов
endif()
```

`PROJECT_IS_TOP_LEVEL` (CMake 3.21+) — удобная переменная, истинная, когда проект собирается сам по себе, и ложная, когда подключён через `add_subdirectory`/`FetchContent`. Так тесты и загрузка тяжёлого GoogleTest автоматически отключаются у того, кто использует твою библиотеку как зависимость.

## Практические правила

Линкуй с целью фреймворка, предоставляющей `main` (`GTest::gtest_main` / `Catch2::Catch2WithMain`), — не пиши `main` вручную без необходимости.

Всегда используй `gtest_discover_tests` / `catch_discover_tests` вместо ручного `add_test` — это регистрирует каждый тест отдельно, и в выводе CTest видно, какой именно тест упал.

Для Catch2 не забудь добавить `${catch2_SOURCE_DIR}/extras` в `CMAKE_MODULE_PATH` перед `include(Catch)` — иначе `catch_discover_tests` не найдётся.

На Windows с GoogleTest ставь `gtest_force_shared_crt ON`, чтобы избежать конфликтов runtime.

Оборачивай тесты в опцию, завязанную на `PROJECT_IS_TOP_LEVEL`, чтобы не навязывать их потребителям.

Фиксируй версию фреймворка тегом в `GIT_TAG` — как и для любой FetchContent-зависимости, ради воспроизводимости.
