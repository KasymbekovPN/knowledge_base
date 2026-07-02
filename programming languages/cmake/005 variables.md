---
tags:
  - programming-language
  - cmake
---
[[programming languages/cmake/_|<=]]

В CMake есть два принципиально разных вида переменных — **обычные** (живут только во время одного запуска конфигурации) и **кэш-переменные** (сохраняются в `CMakeCache.txt` между запусками). `option` — это удобная обёртка для булевой кэш-переменной.

## Обычные переменные: `set`

Живут в пределах текущего запуска CMake и подчиняются области видимости (scope). Создаются и читаются так:

```cmake
set(MY_VAR "hello")
message(STATUS "значение: ${MY_VAR}")   # значение: hello
```

Список значений — это просто строка с разделителями `;`:

```cmake
set(SOURCES main.cpp utils.cpp parser.cpp)
add_executable(app ${SOURCES})
```

Область видимости: переменная видна в текущем каталоге и вложенных, но изменения внутри функции или `add_subdirectory` по умолчанию не «всплывают» наверх. Чтобы изменить переменную в родительской области:

```cmake
set(MY_VAR "value" PARENT_SCOPE)
```

## Кэш-переменные

Сохраняются в `CMakeCache.txt` и переживают повторные запуски `cmake`. Именно их вы задаёте из командной строки через `-D`. Синтаксис требует указания типа и описания:

```cmake
set(MAX_THREADS 4 CACHE STRING "Число потоков для сборки")
```

Типы: `BOOL`, `STRING`, `PATH`, `FILEPATH`. Тип влияет на то, как переменная отображается в GUI (`cmake-gui`, `ccmake`).

Важная особенность: `set(... CACHE ...)` **не перезапишет** уже существующее в кэше значение. Это сделано специально — чтобы значение, заданное пользователем через `-D`, не затиралось при каждой переконфигурации. Значение из командной строки имеет приоритет:

```
cmake -B build -DMAX_THREADS=16
```

Чтобы принудительно перезаписать, добавляют `FORCE` (использовать осторожно):

```cmake
set(MAX_THREADS 8 CACHE STRING "..." FORCE)
```

## `option`

Сокращение для булевой кэш-переменной. Эти две строки эквивалентны:

```cmake
option(BUILD_TESTS "Собирать тесты" ON)
# то же самое, что:
set(BUILD_TESTS ON CACHE BOOL "Собирать тесты")
```

Третий аргумент — значение по умолчанию (`ON`/`OFF`). Переопределяется из командной строки:

```
cmake -B build -DBUILD_TESTS=OFF
```

## Полный пример

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 4.0.0)  
project(Demo LANGUAGES CXX)  
  
# --- option: булевы переключатели сборки ---  
option(BUILD_TESTS "Build module tests" ON)  
option(ENABLE_LOGGING "Enable logging" OFF)  
message(BUILD_TESTS ": ${BUILD_TESTS}")  
message(ENABLE_LOGGING ": ${ENABLE_LOGGING}")  
  
# --- кэш-переменная строкового типа ---  
set(APP_NAME "MyApp" CACHE STRING "App name")  
  
# --- обычная переменная (внутренняя, не в кэше) ---  
set(SOURCES src/main.cpp src/core.cpp)  
add_executable(${APP_NAME} ${SOURCES})  
message("add_executable(${APP_NAME} ${SOURCES})")  
  
# Применяем логирование, если включено  
if (ENABLE_LOGGING)  
    target_compile_definitions(  
            ${APP_NAME}  
            PRIVATE ENABLE_LOGGING  
    )  
    message(STATUS "Logging on")  
endif()  
  
# Собираем тесты, если включено  
if (BUILD_TESTS)  
    message(STATUS "Tests will built")  
    # add_subdirectory(tests)  
    # enable_testing()endif()  
  
message(STATUS "Application name: ${APP_NAME}")
```

### Сборка с настройками по умолчанию
```
cmake -B .build
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
BUILD_TESTS: ON
ENABLE_LOGGING: OFF
add_executable(MyApp src/main.cpp;src/core.cpp)
-- Tests will built
-- Application name: MyApp
-- Configuring done (1.5s)
-- Generating done (0.0s)
-- Build files have been written to: C:/projects/knowledge_base/programming languages/cmake/code/005 variables/.build
```

### Сборка с переопределением
```
cmake -B .build -DBUILD_TESTS=OFF -DENABLE_LOGGING=ON -DAPP_NAME=Server
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
BUILD_TESTS: OFF
ENABLE_LOGGING: ON
add_executable(Server src/main.cpp;src/core.cpp)
-- Logging on
-- Application name: Server
-- Configuring done (1.4s)
-- Generating done (0.0s)
-- Build files have been written to: C:/projects/knowledge_base/programming languages/cmake/code/005 variables/.build
```

## Просмотр и сравнение

|Свойство|Обычная переменная|Кэш-переменная / `option`|
|---|---|---|
|Сохраняется между запусками|Нет|Да (`CMakeCache.txt`)|
|Задаётся через `-D`|Нет|Да|
|Имеет тип и описание|Нет|Да|
|Область видимости|Текущий scope|Глобальная|
|Видна в `cmake-gui`|Нет|Да|

## Полезные команды

Посмотреть все кэш-переменные проекта:

```
cmake -B build -L          # кратко
cmake -B build -LH         # с описаниями
```

Удалить переменную из кэша:

```
cmake -B build -UMAX_THREADS
```

**Практическое правило:** используйте `option`/кэш-переменные для всего, что пользователь должен иметь возможность настроить снаружи (флаги функций, пути, режимы), а обычные `set` — для внутренних, вычисляемых значений, которые не нужно выставлять извне.
