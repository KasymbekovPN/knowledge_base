---
tags:
  - programming-language
  - cmake
---
[[programming languages/cmake/_|<=]]

Установка — это копирование собранных артефактов (библиотек, исполняемых файлов, заголовков) из каталога сборки в постоянное место в системе, откуда ими можно пользоваться. Команда `install` описывает, что и куда копировать, а `CMAKE_INSTALL_PREFIX` задаёт корневой каталог назначения.

## Зачем это нужно

До сих пор всё оставалось внутри `build/`. Но библиотека, которую соберут и бросят в каталоге сборки, бесполезна для других. Установка раскладывает артефакты по стандартным местам (`bin/`, `lib/`, `include/`), чтобы:

- исполняемые файлы попали туда, где их найдёт система;
- библиотеки и заголовки стали доступны другим проектам;
- получился аккуратный, распространяемый набор файлов, отделённый от мусора сборки.

## `CMAKE_INSTALL_PREFIX`

Переменная, задающая корневой каталог, куда всё устанавливается. Значения по умолчанию: `/usr/local` на Unix, `C:/Program Files/<Проект>` на Windows.

Задаётся на этапе конфигурации:

```
cmake -B build -DCMAKE_INSTALL_PREFIX=/opt/myproject
```

Все пути в командах `install` отсчитываются **относительно** этого префикса. Если префикс `/opt/myproject`, а библиотека ставится в `lib`, итоговый путь — `/opt/myproject/lib`.

Для локальных тестов установки удобно указывать префикс внутри проекта, чтобы не трогать системные каталоги и не требовать прав root:

```
cmake -B build -DCMAKE_INSTALL_PREFIX=${PWD}/install
```

## Запуск установки

После сборки:

```
cmake --install build
```

Можно переопределить префикс прямо в момент установки (не переконфигурируя):

```
cmake --install build --prefix /opt/myproject
```

## Команда `install`: основные формы

`install` имеет несколько разновидностей в зависимости от того, что устанавливается.

**`install(TARGETS ...)`** — устанавливает цели (библиотеки, исполняемые файлы):

```cmake
install(TARGETS myapp mylib
    RUNTIME  DESTINATION bin       # исполняемые файлы и .dll
    LIBRARY  DESTINATION lib       # .so динамические библиотеки
    ARCHIVE  DESTINATION lib       # .a/.lib статические библиотеки
)
```

Три типа назначения различают виды артефактов: `RUNTIME` — исполняемые файлы (и DLL на Windows), `LIBRARY` — динамические библиотеки Unix, `ARCHIVE` — статические библиотеки. CMake сам раскладывает каждую цель по нужному назначению.

**`install(FILES ...)`** — отдельные файлы (например, публичные заголовки):

```cmake
install(FILES include/mylib/api.hpp
    DESTINATION include/mylib
)
```

**`install(DIRECTORY ...)`** — целый каталог (удобно для заголовков библиотеки):

```cmake
install(DIRECTORY include/          # обрати внимание на слэш в конце
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"  # только заголовки, без прочего
)
```

Слэш в конце `include/` означает «содержимое каталога», без слэша — «сам каталог». Это тонкость, о которую часто спотыкаются: `include/` копирует то, что _внутри_ `include`, а `include` создаст вложенный `include/` в назначении.

## Стандартные пути установки: `GNUInstallDirs`

Жёстко прописывать `bin`, `lib`, `include` — не лучшая практика, потому что на разных системах соглашения различаются (например, `lib` против `lib64`). Модуль `GNUInstallDirs` даёт стандартные переменные для этих путей:

```cmake
include(GNUInstallDirs)

install(TARGETS mylib
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}       # обычно bin
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}       # обычно lib или lib64
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}            # обычно include
)
```

Эти переменные подстраиваются под платформу и дистрибутив автоматически. Использовать `GNUInstallDirs` — рекомендуемая практика для любого проекта, который планируется устанавливать.

## Полный пример

Структура:

```
mylib/
├── CMakeLists.txt
├── include/
│   └── mylib/
│       └── greeter.hpp
└── src/
    └── greeter.cpp
```

`include/mylib/greeter.hpp`:

```cpp
#pragma once
#include <string>
std::string greet(const std::string& name);
```

`src/greeter.cpp`:

```cpp
#include "mylib/greeter.hpp"
std::string greet(const std::string& name) {
    return "Привет, " + name + "!";
}
```

`CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.16)
project(MyLib VERSION 1.0.0 LANGUAGES CXX)

include(GNUInstallDirs)

add_library(greeter STATIC src/greeter.cpp)
target_include_directories(greeter PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_features(greeter PUBLIC cxx_std_17)

# Установка цели
install(TARGETS greeter
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Установка заголовков
install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
```

Сборка и установка в локальный каталог:

```
cmake -B build -DCMAKE_INSTALL_PREFIX=${PWD}/install
cmake --build build
cmake --install build
```

Результат в `install/`:

```
install/
├── include/
│   └── mylib/
│       └── greeter.hpp
└── lib/
    └── libgreeter.a
```

## Роль `$<INSTALL_INTERFACE>` в путях к заголовкам

Обрати внимание на генераторное выражение в `target_include_directories`:

```cmake
target_include_directories(greeter PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
```

Смысл двух путей в том, что заголовки лежат в **разных местах** до и после установки. Пока библиотека собирается внутри проекта, заголовки в `<исходники>/include` — это описывает `BUILD_INTERFACE`. После установки они окажутся в `<префикс>/include` — это `INSTALL_INTERFACE`. Одна и та же цель корректно сообщает потребителю правильный путь в обоих сценариях. Без этого установленная библиотека указывала бы на путь исходников, которого у пользователя может уже не быть.

## Дополнительные возможности

**Компоненты** — разбиение установки на части (например, отдельно runtime и отдельно файлы для разработчиков):

```cmake
install(TARGETS mylib
    RUNTIME DESTINATION bin COMPONENT runtime
    ARCHIVE DESTINATION lib COMPONENT development
)
```

Установить только компонент:

```
cmake --install build --component runtime
```

**Права доступа** — для файлов, которым нужны конкретные разрешения:

```cmake
install(FILES script.sh
    DESTINATION bin
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ WORLD_READ
)
```

**DESTDIR** — временный префикс поверх основного, нужен при упаковке (пакетные системы дистрибутивов):

```
DESTDIR=/tmp/staging cmake --install build
# установит в /tmp/staging/<CMAKE_INSTALL_PREFIX>/...
```

## Что установка пока НЕ делает

Важное ограничение: описанная выше установка копирует библиотеку и заголовки, но **не** делает так, чтобы другой проект мог подключить её через `find_package(MyLib)`. Для этого нужно дополнительно установить и сгенерировать config-файлы пакета (`MyLibConfig.cmake`) и экспортировать цели. Пока что установленную библиотеку можно использовать только «вручную» — зная пути к `lib` и `include`.

## Практические правила

Всегда используй `GNUInstallDirs` вместо жёстко прописанных `bin`/`lib`/`include` — это делает установку корректной на всех платформах.

Задавай пары путей `$<BUILD_INTERFACE:...>` / `$<INSTALL_INTERFACE:...>` в `target_include_directories`, если библиотеку будут устанавливать, — иначе установленная версия будет ссылаться на несуществующие пути.

Для локальной проверки ставь в каталог внутри проекта (`-DCMAKE_INSTALL_PREFIX=${PWD}/install`) — увидишь результат, не засоряя систему и не требуя root.

Следи за слэшем в конце пути в `install(DIRECTORY include/ ...)` — он определяет, копируется содержимое каталога или сам каталог.

**Следующий логичный шаг** — экспорт целей: научить установленную библиотеку генерировать `MyLibConfig.cmake`, чтобы другие проекты подключали её одной строкой `find_package(MyLib)` с автоматическим получением всех путей и требований, ровно как ты подключаешь fmt или spdlog. Это превращает твою библиотеку в полноценный переиспользуемый пакет и логически завершает тему установки.

---
----
---
---


**5. Структура проектов (2-3 дня)**

- Экспорт целей и генерация config-файлов для своей библиотеки

**6. Тестирование и инструменты (2-3 дня)**

- CTest, `enable_testing`, `add_test`
- Интеграция GoogleTest/Catch2
- `CMakePresets.json` (пресеты конфигурации)

**7. Продвинутые темы (по необходимости)**

- Собственные функции/макросы, модули в `cmake/`
- Кросс-компиляция, toolchain-файлы
- CPack для упаковки
- Кастомные команды и цели (`add_custom_command/target`)
