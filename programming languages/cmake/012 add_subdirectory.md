---
tags:
  - programming-language
  - cmake
---
[[programming languages/cmake/_|<=]]

`add_subdirectory` — команда, которая подключает к сборке другой каталог с собственным `CMakeLists.txt`. Это основной способ организовать многомодульный проект: разбить его на части, каждая со своим файлом сборки, и собрать всё как единое целое.

## Базовая идея

CMake-проект не обязан жить в одном `CMakeLists.txt`. Каждый значимый подкаталог (библиотека, приложение, тесты) получает свой `CMakeLists.txt`, а корневой файл связывает их через `add_subdirectory`:

```cmake
add_subdirectory(имя_каталога)
```

Когда CMake встречает эту команду, он заходит в указанный каталог, читает лежащий там `CMakeLists.txt` и выполняет его. Все объявленные внутри цели (`add_library`, `add_executable`) становятся частью общей сборки.

## Ключевой момент: цели видны глобально

Обрати внимание на важную деталь. Цель `math` объявлена в `math/CMakeLists.txt`, но используется в `app/CMakeLists.txt` — в другом каталоге. Это работает, потому что **имена целей глобальны** для всего проекта. Как только цель создана через `add_library`/`add_executable`, на неё можно ссылаться из любого места сборки по имени.

Именно это делает `target_link_libraries(app PRIVATE math)` возможным: `math` — не путь и не файл, а имя цели, известное всему проекту.

Отсюда следствие: **порядок важен**. Библиотеку нужно подключить (`add_subdirectory(math)`) до того, как приложение попытается на неё сослаться. Если поменять порядок в корневом файле, `app` не найдёт цель `math`.

## Что происходит с переменными и областью видимости

`add_subdirectory` создаёт **новую область видимости** переменных. Это важное отличие от целей:

Обычные переменные (`set`) из корневого файла **видны** в подкаталоге (наследуются вниз).

Но переменные, установленные **внутри** подкаталога, по умолчанию **не всплывают** обратно в родительский. Чтобы «протолкнуть» значение наверх, нужен `PARENT_SCOPE`:

```cmake
# внутри math/CMakeLists.txt
set(MATH_VERSION "1.0" PARENT_SCOPE)   # станет видно в корневом файле
```

Цели этому не подчиняются — они глобальны и видны везде независимо от области видимости переменных. Путаница между «глобальные цели» и «локальные переменные» — частый источник ошибок у начинающих.

## Полезные переменные путей

Внутри каждого `CMakeLists.txt` доступны переменные, указывающие на текущий и корневой каталоги:

`CMAKE_CURRENT_SOURCE_DIR` — каталог с текущим обрабатываемым `CMakeLists.txt`.

`CMAKE_CURRENT_BINARY_DIR` — соответствующий ему каталог сборки.

`CMAKE_SOURCE_DIR` — корень всего проекта (где верхний `CMakeLists.txt`).

`CMAKE_BINARY_DIR` — корень каталога сборки.

Они помогают строить пути, не завязываясь на то, из какого каталога запущен CMake:

```cmake
target_include_directories(math PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
```

## Форма с отдельным каталогом сборки

У `add_subdirectory` есть второй, необязательный аргумент — куда помещать артефакты сборки этого подкаталога:

```cmake
add_subdirectory(source_dir binary_dir)
```

Обычно он не нужен — CMake сам зеркалит структуру. Он требуется в редком случае, когда подключаемый каталог находится **вне** дерева проекта (по абсолютному пути или через `..`), — тогда CMake не может автоматически определить, где размещать его сборку, и путь нужно задать явно:

```cmake
add_subdirectory(/path/to/external/lib external_build)
```

## Управление опциями подключаемого каталога

Как и с `FetchContent` (который под капотом использует `add_subdirectory`), опции подключаемого модуля задаются кэш-переменными до вызова:

```cmake
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)   # отключить тесты в подпроекте
add_subdirectory(external/somelib)
```

Это работает, потому что подкаталог видит переменные родителя, и заданное заранее значение переопределяет его дефолт.

## `EXCLUDE_FROM_ALL`

По умолчанию все цели подкаталога собираются вместе с проектом. Флаг `EXCLUDE_FROM_ALL` исключает их из сборки по умолчанию — они соберутся, только если от них кто-то явно зависит:

```cmake
add_subdirectory(examples EXCLUDE_FROM_ALL)   # примеры не собираются, если не нужны
```

Полезно для примеров, бенчмарков или опциональных компонентов подключаемой библиотеки, которые не хочется собирать всегда.

## Типичная структура среднего проекта

```
project/
├── CMakeLists.txt              # project(), общие настройки, add_subdirectory(...)
├── libs/
│   ├── core/
│   │   └── CMakeLists.txt      # add_library(core ...)
│   └── utils/
│       └── CMakeLists.txt      # add_library(utils ...)
├── apps/
│   └── main_app/
│       └── CMakeLists.txt      # add_executable + link core, utils
└── tests/
    └── CMakeLists.txt          # тесты
```

Корневой файл:

```cmake
cmake_minimum_required(VERSION 3.16)
project(Project LANGUAGES CXX)

add_subdirectory(libs/core)
add_subdirectory(libs/utils)
add_subdirectory(apps/main_app)

enable_testing()
add_subdirectory(tests)
```

Каждый модуль изолирован в своём каталоге, объявляет свои цели и зависимости, а корневой файл только оркестрирует порядок подключения.

## add_subdirectory vs FetchContent vs find_package

Три способа подключить чужой код, каждый для своей ситуации:

`add_subdirectory` — исходники зависимости **уже лежат** внутри твоего дерева проекта (вложенный подкаталог, git submodule, вендоренная копия). CMake встраивает их сборку в твою.

`FetchContent` — исходников нет локально, CMake их **скачивает** и затем внутренне делает тот же `add_subdirectory`.

`find_package` — библиотека **установлена в системе** как готовый бинарник, CMake только находит её.

## Практические правила

Давай каждому логическому модулю (библиотеке, приложению, набору тестов) свой каталог со своим `CMakeLists.txt` — это делает проект читаемым и масштабируемым.

Следи за порядком `add_subdirectory`: цель должна быть объявлена раньше, чем на неё ссылаются. Библиотеки — перед приложениями, которые их используют.

Помни разницу: **цели глобальны**, **переменные локальны** (нужен `PARENT_SCOPE`, чтобы поднять их наверх).

Держи каждый `CMakeLists.txt` ответственным только за свой каталог — обращайся к целям из других каталогов по имени, но не лезь в их внутренние пути и переменные напрямую.

## Простой пример

Структура:

```
myproject/
├── CMakeLists.txt          ← корневой
├── math/
│   ├── CMakeLists.txt      ← модуль библиотеки
│   ├── math.hpp
│   └── math.cpp
└── app/
    ├── CMakeLists.txt      ← модуль приложения
    └── main.cpp
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 4.0.0)  
project(DemoProject LANGUAGES CXX)  
  
add_subdirectory(math)  
add_subdirectory(app)
```

### math/CMakeLists.txt
```cmake
add_library(math STATIC math.cpp)  
target_include_directories(math PUBLIC  
        ${CMAKE_CURRENT_SOURCE_DIR}  
)  
target_compile_features(math PUBLIC cxx_std_20)
```

### app/CMakeLists.txt
```cmake
add_executable(app main.cpp)  
target_link_libraries(app PRIVATE math)  
target_compile_features(app PUBLIC cxx_std_20)
```

### math/math.hpp
```cpp
#pragma once  
int square(int);
```

### math/math.cpp
```cpp
#include <math.hpp>  
  
int square(const int x) {  
    return x * x;  
}
```

### app/main.cpp
```cpp
#include "math.hpp"  
  
#include <iostream>  
#include <format>  
  
int main() {  
    const int A{42};  
    std::cout << std::format("square({}) = {}", A, square(A));  
  
    return 0;  
}
```

```
square(42) = 1764
```
