---
tags:
  - programming-language
  - cmake
---
[[programming languages/cmake/_|<=]]

`FetchContent` — модуль CMake для автоматической загрузки и подключения внешних зависимостей **прямо во время конфигурации**. В отличие от `find_package`, который ищет уже установленную библиотеку, `FetchContent` сам скачивает исходники (обычно из git) и встраивает их в твою сборку.

## Отличие от find_package

Разница принципиальная:

`find_package` — библиотека должна быть **уже установлена** в системе (вручную, через пакетный менеджер, из пакета ОС). CMake только находит её.

`FetchContent` — библиотеки может не быть нигде. CMake **скачает исходники** и соберёт их как часть твоего проекта. Ничего не нужно ставить заранее.

Это делает проект самодостаточным: клонировал репозиторий, запустил `cmake` — зависимости подтянутся сами.

## Базовый механизм

Работа состоит из двух шагов: **объявить** зависимость и **сделать её доступной**.

```cmake
include(FetchContent)               # подключаем модуль

FetchContent_Declare(               # 1. объявляем, откуда брать
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG        10.2.1
)

FetchContent_MakeAvailable(fmt)     # 2. скачиваем и подключаем
```

После `FetchContent_MakeAvailable` цели библиотеки (например, `fmt::fmt`) становятся доступны так, будто они объявлены в твоём проекте — их можно линковать напрямую:

```cmake
target_link_libraries(app PRIVATE fmt::fmt)
```

## Разбор команд

**`FetchContent_Declare`** — описывает источник. Не скачивает ничего, только регистрирует. Основные параметры источника:

```cmake
FetchContent_Declare(
    dep_name
    GIT_REPOSITORY https://github.com/user/repo.git
    GIT_TAG        v1.2.3        # тег, ветка или хеш коммита
)
```

Важно: в `GIT_TAG` предпочтительно указывать **конкретный тег или хеш коммита**, а не ветку вроде `main`. Ветка меняется во времени, и сборка перестаёт быть воспроизводимой. Тег/хеш фиксирует точную версию.

Кроме git, поддерживаются и другие источники — архив по URL:

```cmake
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    URL_HASH SHA256=<хеш>          # проверка целостности архива
)
```

Загрузка из архива по URL обычно **быстрее**, чем клонирование git (не тянется история), поэтому для релизов часто предпочитают её.

**`FetchContent_MakeAvailable`** — выполняет реальную работу: скачивает исходники (если ещё не скачаны) и вызывает `add_subdirectory` для них, встраивая их `CMakeLists.txt` в твою сборку. Именно поэтому цели зависимости становятся доступны напрямую.

## Несколько зависимостей

`FetchContent_MakeAvailable` принимает несколько имён сразу:

```cmake
include(FetchContent)

FetchContent_Declare(fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 10.2.1)

FetchContent_Declare(spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.13.0)

FetchContent_MakeAvailable(fmt spdlog)

add_executable(app main.cpp)
target_link_libraries(app PRIVATE fmt::fmt spdlog::spdlog)
```

## Типичный кейс: GoogleTest

Один из самых частых сценариев `FetchContent` — подключение фреймворка тестов:

```cmake
include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)

# На Windows заставляет gtest использовать общую runtime-библиотеку
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(my_tests test.cpp)
target_link_libraries(my_tests PRIVATE GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(my_tests)
```

## Управление сборкой зависимости

У сторонних библиотек часто есть свои опции (сборка тестов, примеров), которые не нужны при подключении. Их отключают через кэш-переменные **до** `FetchContent_MakeAvailable`:

```cmake
set(FMT_TEST OFF CACHE BOOL "" FORCE)          # не собирать тесты fmt
set(SPDLOG_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(fmt spdlog)
```

Поскольку зависимость подключается через `add_subdirectory`, её опции — это обычные кэш-переменные, и заданные заранее значения переопределяют дефолты библиотеки. Это ускоряет сборку, убирая ненужные цели.

## Плюсы и минусы

**Плюсы:** проект самодостаточен — не нужно ничего ставить заранее; версии зафиксированы в `CMakeLists.txt` и воспроизводимы; работает одинаково на всех платформах; идеально для CI и для того, чтобы «клонировал и собрал».

**Минусы:** зависимости пересобираются из исходников (дольше первая сборка), исходники дублируются в каждом проекте (в `build/_deps`), нет системного кэша между проектами (в отличие от vcpkg/Conan). Для больших зависимостей вроде Qt или Boost это накладно — там уместнее пакетный менеджер.

## FetchContent + find_package: гибридный подход

Современный паттерн — сначала попробовать найти установленную библиотеку, и только если её нет, скачать через FetchContent. Это даёт лучшее из двух миров: используется системная копия, если она есть, иначе — автоматическая загрузка.

```cmake
include(FetchContent)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG        10.2.1
    FIND_PACKAGE_ARGS         # разрешает сначала попробовать find_package(fmt)
)

FetchContent_MakeAvailable(fmt)   # найдёт установленную ИЛИ скачает
```

Параметр `FIND_PACKAGE_ARGS` (CMake 3.24+) заставляет `FetchContent_MakeAvailable` сперва выполнить `find_package`, и только при неудаче — загрузку. В обоих случаях цель `fmt::fmt` окажется доступной.

## Практические правила

Всегда фиксируй версию через тег или хеш коммита в `GIT_TAG` — никогда не `main`/`master`, иначе сборки перестанут быть воспроизводимыми.

Отключай ненужные опции зависимости (тесты, примеры) через кэш-переменные перед `MakeAvailable` — это заметно ускоряет сборку.

Для релизов предпочитай загрузку архивом по `URL` с `URL_HASH` вместо `GIT_REPOSITORY` — быстрее и с проверкой целостности.

Используй `FetchContent` для небольших и средних библиотек и для CI, где важна самодостаточность. Для тяжёлых зависимостей и когда нужен кэш между проектами — рассматривай vcpkg/Conan.

**Как выбрать между тремя механизмами:** `find_package` — библиотека уже стоит в системе; `FetchContent` — хочешь, чтобы проект сам всё скачал и собрал без внешних инструментов; vcpkg/Conan — нужен полноценный менеджер с кэшированием и версионированием для многих проектов сразу. `FIND_PACKAGE_ARGS` позволяет красиво совместить первые два.

## Полный пример

Структура:

```
myproject/
├── CMakeLists.txt
└── main.cpp
```

### main.cpp
```cpp
#include <fmt/core.h>  
  
int main() {  
    fmt::print("Load through FetchContent: {}\n", "fmt");  
}
```

```
Load through FetchContent: fmt
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 4.0.0)  
project(FetchDemo LANGUAGES CXX)  
  
include(FetchContent)  
  
FetchContent_Declare(  
        fmt  
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git        GIT_TAG 10.2.1)  
  
FetchContent_MakeAvailable(fmt)  
  
add_executable(app main.cpp)  
target_link_libraries(app PRIVATE fmt::fmt)  
target_compile_features(app PUBLIC cxx_std_20)
```

При первой конфигурации CMake клонирует `fmt` в `build/_deps/` и настроит его. Повторные запуски используют уже скачанную копию — заново не качает.
