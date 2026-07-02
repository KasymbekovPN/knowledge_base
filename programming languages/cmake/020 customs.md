---
tags:
  - programming-language
  - cmake
---
[[programming languages/cmake/_|<=]]

## Зачем это нужно

Когда в проекте много целей с одинаковыми настройками (флаги предупреждений, стандарт, свойства установки), копирование одних и тех же строк в каждый `CMakeLists.txt` становится проблемой: правку приходится вносить в десяти местах, легко забыть одно. Собственные функции инкапсулируют повторяющуюся логику в одном месте.

---

## `function` и `macro`: базовый синтаксис

CMake даёт два способа определить свою команду — `function` и `macro`. Синтаксис похож:

```cmake
function(my_function arg1 arg2)
    message(STATUS "аргументы: ${arg1}, ${arg2}")
endfunction()

macro(my_macro arg1 arg2)
    message(STATUS "аргументы: ${arg1}, ${arg2}")
endmacro()

my_function(hello world)
my_macro(hello world)
```

Оба вызываются одинаково, но между ними есть **принципиальное различие**, которое определяет, что выбрать.

## Ключевое различие: область видимости

`function` создаёт **новую область видимости переменных** — как отдельный каталог. Переменные, созданные внутри функции, не видны снаружи; чтобы «вернуть» значение, нужен `PARENT_SCOPE`.

`macro` **не создаёт** своей области — он работает так, будто его тело подставлено прямо в место вызова (текстовая подстановка). Переменные, изменённые в макросе, меняются и снаружи.

Наглядно:

```cmake
function(set_in_function)
    set(RESULT "из функции")           # локальна, наружу не видна
endfunction()

macro(set_in_macro)
    set(RESULT "из макроса")           # меняет переменную вызывающего
endmacro()

set_in_function()
message(STATUS "после функции: ${RESULT}")   # пусто

set_in_macro()
message(STATUS "после макроса: ${RESULT}")   # из макроса
```

**Практическое правило:** по умолчанию используй `function` — изоляция области видимости предотвращает случайную порчу переменных вызывающего кода. `macro` бери только когда сознательно нужно менять переменные снаружи или когда `return()` должен возвращать из вызывающего контекста. Неожиданные баги от «протекающих» переменных — самая частая причина боли с макросами.

## Возврат значений из функции

Поскольку функция изолирована, значение возвращают через `PARENT_SCOPE`:

```cmake
function(add_numbers a b out_var)
    math(EXPR result "${a} + ${b}")
    set(${out_var} ${result} PARENT_SCOPE)   # записываем в переменную вызывающего
endfunction()

add_numbers(2 3 sum)
message(STATUS "сумма: ${sum}")   # сумма: 5
```

Приём с `out_var` — распространённый паттерн: имя выходной переменной передаётся аргументом, а функция пишет результат в `${out_var}` с `PARENT_SCOPE`. Так работают многие встроенные команды CMake.

## Аргументы: `ARGN` и `cmake_parse_arguments`

Функции могут принимать переменное число аргументов. Всё, что передано сверх именованных параметров, доступно в списке `ARGN`:

```cmake
function(print_all first)
    message(STATUS "первый: ${first}")
    foreach(item IN LISTS ARGN)           # остальные аргументы
        message(STATUS "  ещё: ${item}")
    endforeach()
endfunction()

print_all(a b c d)
```

Но для серьёзных функций используют `cmake_parse_arguments` — он разбирает именованные аргументы (в стиле самих команд CMake, вроде `PUBLIC`/`PRIVATE`/`DESTINATION`). Это то, что делает свою функцию похожей на встроенную:

```cmake
function(add_my_library)
    set(options STATIC SHARED)                    # флаги (есть/нет)
    set(oneValueArgs NAME)                        # аргументы с одним значением
    set(multiValueArgs SOURCES DEPS)              # аргументы со списком значений
    cmake_parse_arguments(ARG
        "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Теперь доступны ARG_NAME, ARG_SOURCES, ARG_DEPS, ARG_STATIC, ARG_SHARED
    add_library(${ARG_NAME} ${ARG_SOURCES})
    if(ARG_DEPS)
        target_link_libraries(${ARG_NAME} PRIVATE ${ARG_DEPS})
    endif()
endfunction()

# Вызов в стиле встроенных команд CMake:
add_my_library(
    NAME mylib
    SOURCES src/a.cpp src/b.cpp
    DEPS fmt::fmt
)
```

Разберём аргументы `cmake_parse_arguments`: первый (`ARG`) — префикс для результирующих переменных; затем три списка — опции-флаги, одиночные значения, множественные значения; в конце `${ARGN}` — то, что реально передали. После вызова каждый именованный аргумент доступен как `ARG_<ИМЯ>`. Это стандартный способ писать выразительные, самодокументируемые функции.

## Модули в `cmake/`

Определять функции прямо в `CMakeLists.txt` быстро засоряет его. Их выносят в отдельные `.cmake`-файлы в каталоге `cmake/` и подключают через `include()`.

Структура:

```
project/
├── CMakeLists.txt
├── cmake/
│   ├── CompilerWarnings.cmake
│   └── AddMyLibrary.cmake
└── src/...
```

`cmake/CompilerWarnings.cmake`:

```cmake
# Функция, применяющая единый набор предупреждений к цели
function(set_project_warnings target)
    target_compile_options(${target} PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-Wpedantic>
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
    )
endfunction()
```

Подключение и использование в `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.16)
project(MyProject LANGUAGES CXX)

# Путь к своим модулям
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(CompilerWarnings)      # подключает cmake/CompilerWarnings.cmake

add_library(core src/core.cpp)
set_project_warnings(core)     # применяем ко всем нужным целям одной строкой
```

Ключевые моменты:

`list(APPEND CMAKE_MODULE_PATH ...)` — добавляет каталог `cmake/` в пути, где `include()` (и `find_package` в module mode) ищет файлы. Без этого `include(CompilerWarnings)` не найдёт модуль.

`include(CompilerWarnings)` — подключается **по имени без расширения** (CMake сам добавит `.cmake` и поищет в `CMAKE_MODULE_PATH`). Файл выполняется в текущей области, определённые в нём функции становятся доступны дальше.

## `include()` vs `add_subdirectory()`

Важное различие, которое часто путают:

`include(файл.cmake)` — выполняет содержимое файла **в текущей области видимости**, как если бы код был вписан прямо сюда. Не создаёт новый scope. Используется для подключения функций и настроек.

`add_subdirectory(каталог)` — заходит в каталог, выполняет его `CMakeLists.txt` **в новой области видимости**. Используется для подключения модулей проекта с их собственными целями.

Правило простое: функции и переменные-настройки подключай через `include()` из `.cmake`-файлов; подпроекты с целями — через `add_subdirectory()` из каталогов с `CMakeLists.txt`.

## Практический пример: модуль-хелпер для тестов

Соберём пройденное — вынесем создание теста в переиспользуемую функцию.

`cmake/AddTest.cmake`:

```cmake
function(add_unit_test)
    set(oneValueArgs NAME)
    set(multiValueArgs SOURCES DEPS)
    cmake_parse_arguments(ARG "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_executable(${ARG_NAME} ${ARG_SOURCES})
    target_link_libraries(${ARG_NAME} PRIVATE ${ARG_DEPS} GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(${ARG_NAME})
endfunction()
```

`tests/CMakeLists.txt`:

```cmake
add_unit_test(
    NAME calc_test
    SOURCES calc_test.cpp
    DEPS calc
)

add_unit_test(
    NAME parser_test
    SOURCES parser_test.cpp
    DEPS parser
)
```

Вместо шести строк на каждый тест (создать бинарник, слинковать, подключить модуль, зарегистрировать) — один выразительный вызов. Добавить новый тест теперь тривиально, а изменить логику всех тестов можно в одном месте.

## Отладка функций

Полезные приёмы при написании своих команд:

`message(STATUS "...")` внутри функции — печать промежуточных значений. Для отладки удобны уровни: `message(DEBUG "...")` показывается только при `cmake --log-level=DEBUG`.

Проверка, что аргумент передан:

```cmake
if(NOT ARG_NAME)
    message(FATAL_ERROR "add_unit_test: не указан NAME")
endif()
```

`ARG_UNPARSED_ARGUMENTS` (из `cmake_parse_arguments`) содержит аргументы, которые функция не распознала — полезно для проверки опечаток в вызове.

## Практические правила

Предпочитай `function` макросам — изоляция области видимости избавляет от труднонаходимых багов с «протекающими» переменными. `macro` — только когда осознанно нужно менять переменные вызывающего.

Для функций с несколькими параметрами используй `cmake_parse_arguments` — это делает вызовы читаемыми и похожими на встроенные команды CMake.

Выноси функции и общие настройки в `.cmake`-файлы в каталоге `cmake/`, подключай через `include()` после добавления пути в `CMAKE_MODULE_PATH`. Держи `CMakeLists.txt` чистыми.

Различай `include()` (код в текущую область, для функций) и `add_subdirectory()` (новая область, для целей).

Проверяй обязательные аргументы через `if(NOT ARG_...)` и `FATAL_ERROR` — понятная ошибка лучше загадочного сбоя дальше по коду.

**Следующий логичный шаг** плана — кросс-компиляция через toolchain-файлы: как собирать под другую платформу (ARM, embedded, Android), задавая компилятор и окружение целевой системы. Это опирается на понимание переменных и кэша, которое у тебя уже есть, и естественно расширяет тему конфигурации сборки.

----
---
---
---


**7. Продвинутые темы (по необходимости)**

- Кросс-компиляция, toolchain-файлы
- CPack для упаковки
- Кастомные команды и цели (`add_custom_command/target`)
