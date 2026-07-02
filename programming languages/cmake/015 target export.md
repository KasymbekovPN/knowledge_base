---
tags:
  - programming-language
  - cmake
---
[[programming languages/cmake/_|<=]]

Это финальный шаг превращения библиотеки в полноценный переиспользуемый пакет. Цель — чтобы другой проект мог подключить твою библиотеку одной строкой `find_package(MyLib)` и слинковаться с `MyLib::greeter`, автоматически получив все пути и требования — ровно так, как ты подключаешь fmt или spdlog.

## Что было и чего не хватало

В прошлой теме установка копировала библиотеку и заголовки в систему, но `find_package(MyLib)` их не находил — потому что не было **config-файла пакета** (`MyLibConfig.cmake`), который описывает пакет для CMake. Экспорт целей решает именно это: он генерирует файлы, где записаны цели библиотеки, их пути (после установки) и usage requirements.

## Три составные части

Чтобы библиотека стала находимой через `find_package`, нужно установить и сгенерировать три вещи:

Экспорт целей — файл, описывающий сами цели (`MyLibTargets.cmake`), их тип, расположение, публичные свойства.

Config-файл пакета — точка входа для `find_package` (`MyLibConfig.cmake`), который подключает экспорт целей и, при необходимости, ищет зависимости.

Файл версии — описание версии пакета (`MyLibConfigVersion.cmake`), чтобы работала проверка версий в `find_package(MyLib 1.0)`.

## Разберём по шагам

Возьмём библиотеку `greeter` из прошлой темы([[014 install]]) и сделаем её экспортируемой.

### Шаг 1: установка цели с привязкой к EXPORT

К `install(TARGETS ...)` добавляется ключевое слово `EXPORT`, которое присваивает цели имя набора экспорта:

```cmake
include(GNUInstallDirs)

install(TARGETS greeter
    EXPORT MyLibTargets                    # ← цель попадает в набор экспорта
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
```

`EXPORT MyLibTargets` регистрирует цель `greeter` в наборе с именем `MyLibTargets` — этот набор мы затем установим отдельной командой. `INCLUDES DESTINATION` записывает в экспортируемую цель путь к заголовкам, чтобы потребители после установки получали его автоматически.

### Шаг 2: установка самого набора экспорта

Отдельная команда `install(EXPORT ...)` генерирует и устанавливает файл с описанием целей:

```cmake
install(EXPORT MyLibTargets
    FILE MyLibTargets.cmake                # имя генерируемого файла
    NAMESPACE MyLib::                      # цели получат префикс MyLib::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MyLib
)
```

Здесь происходит важное. `NAMESPACE MyLib::` означает, что в экспортированном файле цель `greeter` будет доступна как `MyLib::greeter` — тот самый namespace-префикс, что даёт единообразие и защиту от опечаток. `DESTINATION .../cmake/MyLib` — стандартное место, где `find_package` в config mode ищет файлы пакета (`<prefix>/lib/cmake/MyLib/`).

### Шаг 3: генерация config-файла пакета

`MyLibConfig.cmake` — точка входа для `find_package`. Обычно его делают из шаблона. Создаём файл-шаблон `cmake/MyLibConfig.cmake.in`:

```cmake
@PACKAGE_INIT@

include("${CMAKE_CURRENT_LIST_DIR}/MyLibTargets.cmake")

check_required_components(MyLib)
```

`@PACKAGE_INIT@` — плейсхолдер, который CMake заменит служебным кодом инициализации. Строка `include(...MyLibTargets.cmake)` подключает экспортированные цели. Если у библиотеки есть свои зависимости, здесь же их находят через `find_dependency` (об этом ниже).

Затем в `CMakeLists.txt` превращаем шаблон в готовый config-файл:

```cmake
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/MyLibConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/MyLibConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MyLib
)
```

`configure_package_config_file` подставляет `@PACKAGE_INIT@` и генерирует итоговый `MyLibConfig.cmake` в каталоге сборки.

### Шаг 4: генерация файла версии

Чтобы работала проверка версий (`find_package(MyLib 1.0 REQUIRED)`), генерируем `MyLibConfigVersion.cmake`:

```cmake
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/MyLibConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)
```

`COMPATIBILITY` задаёт правило совместимости: `SameMajorVersion` означает, что пакет версии 1.5 удовлетворит запрос `find_package(MyLib 1.0)`, но не `find_package(MyLib 2.0)` — классическая семантика семантического версионирования. Другие варианты: `ExactVersion`, `SameMinorVersion`, `AnyNewerVersion`.

### Шаг 5: установка config-файлов

Наконец, устанавливаем оба сгенерированных файла туда же, где лежат экспортированные цели:

```cmake
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/MyLibConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/MyLibConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MyLib
)
```

## Пример

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 4.0.0)  
project(MyLib VERSION 1.0.0 LANGUAGES CXX)  
  
include(GNUInstallDirs)  
include(CMakePackageConfigHelpers)  
  
# --- Цель ---  
add_library(greeter STATIC src/greeter.cpp)  
# алиас для использования внутри проекта  
add_library(MyLib::greeter ALIAS greeter)  
target_include_directories(greeter PUBLIC  
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>  
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>  
)  
target_compile_features(greeter PUBLIC cxx_std_20)  
  
# --- Установка цели + регистрация в наборе экспорта ---  
install(TARGETS greeter  
        EXPORT MyLibTargets  
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}  
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}  
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}  
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}  
)  
  
# --- Установка заголовков ---  
install(DIRECTORY include/  
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}  
)  
  
# --- Установка набора экспорта (генерирует MyLibTargets.cmake) ---  
install(EXPORT MyLibTargets  
        FILE MyLibTargets.cmake  
        NAMESPACE MyLib::  
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MyLib  
)  
  
# --- Генерация Config и ConfigVersion ---  
configure_package_config_file(  
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/MuLibConfig.cmake.in  
        ${CMAKE_CURRENT_BINARY_DIR}/MyLibConfig.cmake  
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MyLib  
)  
write_basic_package_version_file(  
        ${CMAKE_CURRENT_BINARY_DIR}/MyLibConfigVersion.cmake  
        VERSION ${PROJECT_VERSION}  
        COMPATIBILITY SameMajorVersion  
)  
# --- Установка Config-файлов ---  
install(FILES  
        ${CMAKE_CURRENT_BINARY_DIR}/MyLibConfig.cmake  
        ${CMAKE_CURRENT_BINARY_DIR}/MyLibConfigVersion.cmake  
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mylib  
)
```

### cmake/MyLibConfig.cmake.in
```cmake
@PACKAGE_INIT@  
  
include("${CMAKE_CURRENT_LIST_DIR}/MyLibTargets.cmake")  
  
check_required_components(MyLib)
```

## Результат установки

```
cmake -B .build -DCMAKE_INSTALL_PREFIX="C:\projects\knowledge_base\programming languages\cmake\code\015 target export\lib\install"
cmake --build .build --config Release
cmake --install .build
```

Раскладка в `install/`:

```
install/
├── include/
│   └── mylib/greeter.hpp
└── lib/
    ├── libgreeter.a
    └── cmake/
        └── MyLib/
            ├── MyLibConfig.cmake          # точка входа find_package
            ├── MyLibConfigVersion.cmake   # проверка версии
            └── MyLibTargets.cmake         # описание целей
```

## Использование из другого проекта

Теперь любой проект подключает библиотеку стандартно:

### main.cpp
```cpp
/*  
cmake -B .build -DCMAKE_PREFIX_PATH="C:\projects\knowledge_base\programming languages\cmake\code\015 target export\lib\install"  
cmake --build .build --config Release */  
#include <myLib/greeter.hpp>  
  
#include <iostream>  
  
int main() {  
    auto s = std::string("WORLD");  
    std::cout << greet(s) << std::endl;  
}
```

```
Hello, WORLD!
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 4.0.0)  
project(Consumer LANGUAGES CXX)  
  
find_package(MyLib 1.0 REQUIRED)  
  
add_executable(app main.cpp)  
# путь к заголовкам и C++17 придут сами  
target_link_libraries(app PRIVATE MyLib::greeter)
```

`app` автоматически получит путь к заголовкам `greeter` и требование C++17 — всё это записано в экспортированной цели `MyLib::greeter`. Именно так работают fmt, spdlog и любая современная CMake-библиотека.

## Если у библиотеки есть свои зависимости

Когда твоя библиотека сама зависит от чего-то (например, публично линкуется с fmt), это нужно отразить в config-файле, иначе у потребителя отвалится линковка. В шаблоне `MyLibConfig.cmake.in`:

```cmake
@PACKAGE_INIT@

include(CMakeFindDependencyMacro)
find_dependency(fmt 9.0)                    # найти зависимость у потребителя

include("${CMAKE_CURRENT_LIST_DIR}/MyLibTargets.cmake")

check_required_components(MyLib)
```

`find_dependency` — обёртка над `find_package`, предназначенная для config-файлов. Смысл в том, что зависимости твоей библиотеки должны быть найдены и у того, кто её использует. Без этой строки потребитель получит ошибку про ненайденные цели fmt при линковке.

## Экспорт из дерева сборки (без установки)

Есть и второй вид экспорта — прямо из каталога сборки, без установки в систему, командой `export(EXPORT ...)`. Он позволяет использовать библиотеку из её `build/` каталога. На практике это нужно редко (в основном для сложных суперсборок), и для большинства проектов достаточно экспорта при установке, описанного выше.

## Практические правила

Всегда используй `NAMESPACE` в `install(EXPORT ...)` — потребители будут писать `MyLib::greeter`, что единообразно с ALIAS-целью внутри проекта и защищает от опечаток.

Обязательно объявляй зависимости через `find_dependency` в config-шаблоне, если библиотека публично зависит от других пакетов, — иначе сборка сломается у потребителя.

Используй `configure_package_config_file` и `write_basic_package_version_file` из модуля `CMakePackageConfigHelpers`, а не пиши config-файлы вручную — они корректно обрабатывают перемещаемость пакета и версии.

Задавай осмысленный `COMPATIBILITY` в файле версии — `SameMajorVersion` подходит большинству библиотек, следующих семантическому версионированию.

Проверяй результат, реально подключив пакет из отдельного тестового проекта через `find_package` с `CMAKE_PREFIX_PATH` на каталог установки — это лучший способ убедиться, что экспорт настроен верно.
