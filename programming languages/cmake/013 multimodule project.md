---
tags:
  - programming-language
  - cmake
---
[[programming languages/cmake/_|<=]]

Это тема о том, как из отдельных команд (`add_subdirectory`, `add_library`, target-команды) собрать связную структуру реального проекта — такую, которая не разваливается при росте и понятна другим разработчикам. Здесь важны не новые команды, а принципы организации.

## Опорный принцип: модуль = каталог = цель

Базовая единица организации — **модуль**: логически цельная часть проекта (библиотека, приложение, набор тестов). Хорошая практика — чтобы один модуль соответствовал одному каталогу с собственным `CMakeLists.txt` и порождал одну основную цель. Это даёт изоляцию: каждый модуль отвечает только за себя, а корневой файл лишь оркеструет их.

## Рекомендуемая структура

Типовая раскладка для проекта с библиотекой ядра, приложением и тестами:

```
project/
├── CMakeLists.txt              # корневой: project(), общие настройки, подключение модулей
├── cmake/                      # вспомогательные .cmake модули, helper-функции
├── include/
│   └── project/                # публичные заголовки (namespace-каталог)
│       └── core.hpp
├── src/                        # реализация библиотеки
│   ├── CMakeLists.txt
│   └── core.cpp
├── apps/                       # исполняемые приложения
│   └── main/
│       ├── CMakeLists.txt
│       └── main.cpp
├── tests/                      # тесты
│   ├── CMakeLists.txt
│   └── core_test.cpp
└── third_party/                # вендоренные зависимости (submodules и т.п.)
```

Ключевые решения в этой структуре:

**Публичные заголовки в `include/project/`** — вложенный каталог с именем проекта. Это даёт «неймспейс на уровне файловой системы»: пользователь пишет `#include <project/core.hpp>`, и такие include не конфликтуют с заголовками других библиотек. Приватные заголовки (детали реализации) держат рядом с исходниками в `src/`.

**Разделение `src` / `apps` / `tests`** — библиотечный код, приложения и тесты физически разнесены. Библиотека не знает о приложениях; приложения и тесты зависят от библиотеки, но не наоборот.

**`cmake/`** — сюда кладут собственные `.cmake` файлы: helper-функции, свои find-модули, наборы общих настроек. Подключаются через `include()` или `CMAKE_MODULE_PATH`.

## Разделение ответственности между файлами

Корневой `CMakeLists.txt` отвечает за проект в целом: объявляет `project()`, задаёт общие политики и опции, подключает модули в правильном порядке. Он **не** должен содержать деталей отдельных целей.

```cmake
cmake_minimum_required(VERSION 3.16)
project(MyProject VERSION 1.0.0 LANGUAGES CXX)

# Дефолт конфигурации
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

# Проектные опции
option(MYPROJECT_BUILD_TESTS "Собирать тесты" ON)
option(MYPROJECT_BUILD_APPS  "Собирать приложения" ON)

# Путь к своим cmake-модулям
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Подключение модулей — порядок: библиотека раньше её потребителей
add_subdirectory(src)

if(MYPROJECT_BUILD_APPS)
    add_subdirectory(apps/main)
endif()

if(MYPROJECT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
```

Модульный `CMakeLists.txt` отвечает только за свои цели — их исходники, зависимости, свойства:

`src/CMakeLists.txt` (библиотека):

```cmake
add_library(core STATIC core.cpp)
add_library(MyProject::core ALIAS core)          # алиас с namespace

target_include_directories(core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(core PUBLIC cxx_std_17)
```

`apps/main/CMakeLists.txt` (приложение):

```cmake
add_executable(main_app main.cpp)
target_link_libraries(main_app PRIVATE MyProject::core)
```

`tests/CMakeLists.txt` (тесты):

```cmake
add_executable(core_test core_test.cpp)
target_link_libraries(core_test PRIVATE MyProject::core)
add_test(NAME core_test COMMAND core_test)
```

## Приём: ALIAS-цели с namespace

Обрати внимание на строку `add_library(MyProject::core ALIAS core)`. Она создаёт альтернативное имя цели с namespace-префиксом `MyProject::`. Смысл в единообразии: тот же самый префикс появляется, когда библиотеку подключают снаружи через `find_package` и линкуют как `MyProject::core`.

Благодаря алиасу и внутренние потребители пишут `target_link_libraries(... MyProject::core)`, и внешние — одинаково. Плюс двойное двоеточие подсказывает CMake, что это цель, а не путь: опечатка в имени даст понятную ошибку на этапе конфигурации, а не при линковке.

## Связи между модулями строятся только через цели

Главное правило здоровой структуры: модули общаются **исключительно через цели и их usage requirements**, а не через глобальные переменные или явные пути.

Правильно — потребитель линкуется с целью и автоматически получает её публичные заголовки, флаги, стандарт:

```cmake
target_link_libraries(main_app PRIVATE MyProject::core)
# путь к include/, C++17 и прочее придут сами от core
```

Неправильно — лезть в чужой каталог руками:

```cmake
# ❌ хрупко: приложение знает внутреннюю раскладку библиотеки
target_include_directories(main_app PRIVATE ${CMAKE_SOURCE_DIR}/src)
```

Первый подход означает, что можно переставлять и переименовывать файлы внутри модуля, не трогая его потребителей — они зависят от цели, а не от структуры каталогов.

## Управление порядком и зависимостями

Поскольку цели глобальны, но должны быть объявлены до использования, порядок `add_subdirectory` в корневом файле отражает граф зависимостей: сначала базовые библиотеки, затем те, что зависят от них, в конце — приложения и тесты. Для проекта из нескольких библиотек:

```cmake
add_subdirectory(libs/base)      # ни от кого не зависит
add_subdirectory(libs/network)   # зависит от base
add_subdirectory(libs/engine)    # зависит от base и network
add_subdirectory(apps/server)    # зависит от engine
```

## Проектные опции с префиксом

Опции проекта стоит именовать с префиксом (`MYPROJECT_BUILD_TESTS`, а не просто `BUILD_TESTS`). Это критично, когда твой проект подключают как подпроект через `add_subdirectory` или `FetchContent`: без префикса твоя опция `BUILD_TESTS` столкнётся с одноимённой опцией родительского проекта или другой зависимости. Префикс делает опции однозначными.

Полезный приём — собирать тесты только когда проект собирается сам по себе, а не как зависимость:

```cmake
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    # мы — корневой проект, а не подключённый через add_subdirectory
    option(MYPROJECT_BUILD_TESTS "Собирать тесты" ON)
else()
    option(MYPROJECT_BUILD_TESTS "Собирать тесты" OFF)
endif()
```

Так тесты твоей библиотеки не будут собираться у тех, кто просто использует её как зависимость.

## Вынесение общих настроек в INTERFACE-библиотеку

Когда несколько модулей должны разделять одинаковые флаги (предупреждения, стандарт, санитайзеры), не дублируй их и не используй глобальные команды. Создай INTERFACE-библиотеку с настройками и линкуй её к нужным целям:

```cmake
# cmake или отдельный модуль
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_17)

add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)
```

Затем каждый модуль подключает их:

```cmake
target_link_libraries(core PRIVATE project_warnings PUBLIC project_options)
```

Это явно, контролируемо и переносит настройки через тот же механизм целей, без глобального «протекания».

## Практические правила

Один модуль — один каталог — один `CMakeLists.txt` — одна основная цель. Не сваливай несвязанные вещи в один файл.

Публичные заголовки — в `include/<имя_проекта>/`, приватные — рядом с исходниками. Это разделяет API и реализацию на уровне файловой системы.

Модули связывай только через цели и их usage requirements; никогда не тянись в чужие каталоги напрямую.

Давай библиотекам ALIAS с namespace (`Proj::lib`) — единообразие между внутренним и внешним использованием плюс защита от опечаток.

Именуй проектные опции с префиксом и отключай тесты/примеры, когда проект подключён как зависимость.

Общие настройки выноси в INTERFACE-библиотеки, а не в глобальные команды.

Держи корневой файл тонким: он оркеструет модули и задаёт общее, но не занимается деталями отдельных целей.

**Следующий практический шаг:** возьми любой свой проект и реорганизуй его в эту структуру — вынеси ядро в `src/` как библиотеку с ALIAS-целью, приложение в `apps/`, тесты в `tests/`, публичные заголовки в `include/<проект>/`. Когда структура заработает, естественным продолжением станет тема `install` и экспорта целей: она превращает твою библиотеку в устанавливаемый пакет, который другие проекты смогут подключать через `find_package(MyProject)` ровно так же, как ты подключаешь fmt или spdlog.

## Полный пример

```
project/
├── CMakeLists.txt              # корневой: project(), общие настройки, подключение модулей
├── include/
│   └── project/                # публичные заголовки (namespace-каталог)
│       └── core.hpp
├── src/                        # реализация библиотеки
│   ├── CMakeLists.txt
│   └── core.cpp
├── options/
│   └── CMakeLists.txt
├── apps/                       # исполняемые приложения
│   └── main/
│       ├── CMakeLists.txt
│       └── main.cpp
├── tests/                      # тесты
│   ├── CMakeLists.txt
│   └── core_test.cpp
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 4.0.0)  
project(MyProject VERSION 1.0.0 LANGUAGES CXX)  
  
# Дефолт конфигурации  
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)  
    set(CMAKE_BUILD_TYPE  Release CACHE STRING "" FORCE)  
endif()  
  
# Проектные опции  
option(MYPROJECT_BUILD_TESTS "Build tests" OFF)  
option(MYPROJECT_BUILD_APPS "Build app" ON)  
  
# Путь к своим cmake-модулям  
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)  
  
# Подключение модулей — порядок: библиотека раньше её потребителей  
add_subdirectory(options)  
add_subdirectory(src)  
  
if(MYPROJECT_BUILD_APPS)  
    add_subdirectory(apps/main)  
endif()  
  
if(MYPROJECT_BUILD_TESTS)  
    enable_testing()  
    add_subdirectory(tests)  
endif()
```

### options/CMakeLists.txt
```cmake
add_library(project_options INTERFACE)  
target_compile_features(project_options INTERFACE cxx_std_20)  
  
add_library(project_warnings INTERFACE)  
target_compile_options(project_warnings INTERFACE  
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-Wpedantic>  
        $<$<CXX_COMPILER_ID:MSVC>:/W4>)
```

### src/CMakeLists.txt
```cmake
add_library(core STATIC core.cpp)  
# алиас с namespace  
add_library(MyProject::core ALIAS core)  
  
target_include_directories(core PUBLIC  
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>  
        $<INSTALL_INTERFACE:include>)  
  
#target_compile_features(core PUBLIC cxx_std_17)  
target_link_libraries(core PUBLIC project_options PRIVATE project_warnings)
```

### apps/main/CMakeLists.txt
```cmake
add_executable(main_app main.cpp)  
target_link_libraries(main_app PRIVATE MyProject::core)
```

### include/project/core.hpp
```cpp
#pragma once  
  
int square(int);
```

### src/core.cpp
```cpp
#include <project/core.hpp>  
  
int square(const int x) { return x * x; }
```

### apps/main/main.cpp
```cpp
#include <project/core.hpp>  
  
#include <iostream>  
#include <format>  
  
int main() {  
    const int A{42};  
    std::cout << std::format("square({}) = {}", A, square(42));  
}
```

```
square(42) = 1764
```
