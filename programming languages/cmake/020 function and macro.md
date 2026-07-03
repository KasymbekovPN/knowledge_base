---
tags:
  - programming-language
  - cmake
---
[[programming languages/cmake/_|<=]]

## Зачем это нужно

Когда в проекте много целей с одинаковыми настройками (флаги предупреждений, стандарт, свойства установки), копирование одних и тех же строк в каждый `CMakeLists.txt` становится проблемой: правку приходится вносить в десяти местах, легко забыть одно. Собственные функции инкапсулируют повторяющуюся логику в одном месте.

## `function` и `macro`: базовый синтаксис

CMake даёт два способа определить свою команду — `function` и `macro`. Синтаксис похож:

```cmake
cmake_minimum_required(VERSION 4.0.0)  
project(proj LANGUAGES CXX)  
  
function(my_func arg0 arg1)  
    message(STATUS "F ARGS: ${arg0} ${arg1}")  
endfunction()  
  
macro(my_macro arg0 arg1)  
    message(STATUS "M ARGS: ${arg0} ${arg1}")  
endmacro()  
  
my_func(hello world)  
my_macro(hello world)
```

```
-- Selecting Windows SDK version 10.0.26100.0 to target Windows 10.0.26200.
-- F ARGS: hello world
-- M ARGS: hello world
-- Configuring done (0.0s)
-- Generating done (0.0s)
-- Build files have been written to: C:/projects/knowledge_base/programming languages/cmake/code/020 customs/000 simple demo/build
```

Оба вызываются одинаково, но между ними есть **принципиальное различие**, которое определяет, что выбрать.

## Ключевое различие: область видимости

`function` создаёт **новую область видимости переменных** — как отдельный каталог. Переменные, созданные внутри функции, не видны снаружи; чтобы «вернуть» значение, нужен `PARENT_SCOPE`.

`macro` **не создаёт** своей области — он работает так, будто его тело подставлено прямо в место вызова (текстовая подстановка). Переменные, изменённые в макросе, меняются и снаружи.

Наглядно:

```cmake
cmake_minimum_required(VERSION 4.0.0)  
project(proj LANGUAGES CXX)  
  
function(set_in_func)  
    set(RESULT "from-function")  
endfunction()  
  
macro(set_in_macro)  
    set(RESULT "from-macro")  
endmacro()  
  
set_in_func()  
message(STATUS "after function calling [${RESULT}]")  
  
set_in_macro()  
message(STATUS "after macro calling [${RESULT}]")
```

```
-- Building for: Visual Studio 18 2026
-- Selecting Windows SDK version 10.0.26100.0 to target Windows 10.0.26200.
-- The CXX compiler identification is MSVC 19.51.36247.0
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: C:/Program Files/Microsoft Visual Studio/18/Community/VC/Tools/MSVC/14.51.36231/bin/Hostx64/x64/cl.exe - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- after function calling []
-- after macro calling [from-macro]
-- Configuring done (2.3s)
-- Generating done (0.0s)
-- Build files have been written to: C:/projects/knowledge_base/programming languages/cmake/code/020 customs/001 func macro diff/.build
```

**Практическое правило:** по умолчанию используй `function` — изоляция области видимости предотвращает случайную порчу переменных вызывающего кода. `macro` бери только когда сознательно нужно менять переменные снаружи или когда `return()` должен возвращать из вызывающего контекста. Неожиданные баги от «протекающих» переменных — самая частая причина боли с макросами.

## Возврат значений из функции

Поскольку функция изолирована, значение возвращают через `PARENT_SCOPE`:

```cmake
cmake_minimum_required(VERSION 4.0.0)  
project(proj LANGUAGES CXX)  
  
function(add_numbers a b out_var)  
    math(EXPR result "${a} + ${b}")  
    # записываем в переменную вызывающего  
    set(${out_var} ${result} PARENT_SCOPE)  
endfunction()  
  
add_numbers(2 3 sum)  
message(STATUS "sum: ${sum}")
```

```
-- Building for: Visual Studio 18 2026
-- Selecting Windows SDK version 10.0.26100.0 to target Windows 10.0.26200.
-- The CXX compiler identification is MSVC 19.51.36247.0
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: C:/Program Files/Microsoft Visual Studio/18/Community/VC/Tools/MSVC/14.51.36231/bin/Hostx64/x64/cl.exe - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- sum: 5
-- Configuring done (1.5s)
-- Generating done (0.0s)
-- Build files have been written to: C:/projects/knowledge_base/programming languages/cmake/code/020 customs/002 func and parent_scope/.build
```

Приём с `out_var` — распространённый паттерн: имя выходной переменной передаётся аргументом, а функция пишет результат в `${out_var}` с `PARENT_SCOPE`. Так работают многие встроенные команды CMake.

## Аргументы: `ARGN` и `cmake_parse_arguments`

Функции могут принимать переменное число аргументов. Всё, что передано сверх именованных параметров, доступно в списке `ARGN`:

```cmake
cmake_minimum_required(VERSION 4.0.0)  
project(proj LANGUAGES CXX)  
  
function(print_all first)  
    message(STATUS "first: ${first}")  
    foreach (item IN LISTS ARGN)  
        message(STATUS " and ${item}")  
    endforeach ()  
endfunction()  
  
print_all(a b c d f)
```

```
-- Building for: Visual Studio 18 2026
-- Selecting Windows SDK version 10.0.26100.0 to target Windows 10.0.26200.
-- The CXX compiler identification is MSVC 19.51.36247.0
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: C:/Program Files/Microsoft Visual Studio/18/Community/VC/Tools/MSVC/14.51.36231/bin/Hostx64/x64/cl.exe - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- first: a
--  and b
--  and c
--  and d
--  and f
-- Configuring done (1.4s)
-- Generating done (0.0s)
-- Build files have been written to: C:/projects/knowledge_base/programming languages/cmake/code/020 customs/003 argn/.build
```

Но для серьёзных функций используют `cmake_parse_arguments` — он разбирает именованные аргументы (в стиле самих команд CMake, вроде `PUBLIC`/`PRIVATE`/`DESTINATION`). Это то, что делает свою функцию похожей на встроенную:

```cmake
cmake_minimum_required(VERSION 4.0.0)  
project(proj LANGUAGES CXX)  
  
function(add_my_library)  
    # флаги (есть/нет)  
    set(options STATIC SHARED)  
    # аргументы с одним значением  
    set(oneValueArgs NAME)  
    # аргументы со списком значений  
    set(multiValueArgs SOURCES DEPS)  
    cmake_parse_arguments(  
            ARG  
            "${options}"  
            "${oneValueArgs}"  
            "${multiValueArgs}"  
            ${ARGN}  
    )  
  
    message(STATUS "ARG_NAME: ${ARG_NAME}")  
    message(STATUS "ARG_SOURCES: ${ARG_SOURCES}")  
    message(STATUS "ARG_DEPS: ${ARG_DEPS}")  
    message(STATUS "ARG_STATIC: ${ARG_STATIC}")  
    message(STATUS "ARG_SHARED: ${ARG_SHARED}")  
endfunction()  
  
# Вызов в стиле встроенных команд CMake:  
add_my_library(  
        NAME mylib  
        SOURCES src/a.cpp src/b.cpp
        DEPS fmt::fmt
)
```

```
-- Building for: Visual Studio 18 2026
-- Selecting Windows SDK version 10.0.26100.0 to target Windows 10.0.26200.
-- The CXX compiler identification is MSVC 19.51.36247.0
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: C:/Program Files/Microsoft Visual Studio/18/Community/VC/Tools/MSVC/14.51.36231/bin/Hostx64/x64/cl.exe - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- ARG_NAME: mylib
-- ARG_SOURCES: src/a.cpp;src/b.cpp
-- ARG_DEPS: fmt::fmt
-- ARG_STATIC: FALSE
-- ARG_SHARED: FALSE
-- Configuring done (1.6s)
-- Generating done (0.0s)
-- Build files have been written to: C:/projects/knowledge_base/programming languages/cmake/code/020 function and macro/004 cmake_parse_arguments/.build
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
