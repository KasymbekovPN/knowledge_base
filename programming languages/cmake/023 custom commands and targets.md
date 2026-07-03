---
tags:
  - programming-language
  - cmake
---
[[programming languages/cmake/_|<=]]

Это механизм для выполнения произвольных действий во время сборки — того, что не покрывается обычными `add_executable`/`add_library`: генерация кода, обработка ресурсов, копирование файлов, запуск скриптов. Две команды — `add_custom_command` и `add_custom_target` — решают разные задачи и часто работают в паре.

## Зачем это нужно

Реальные проекты не сводятся к «скомпилировать .cpp в бинарник». Часто нужно: сгенерировать исходник из описания (протобуферы, парсеры), встроить ресурсы (шейдеры, картинки) в программу, скопировать конфиги рядом с бинарником, запустить кодогенератор перед компиляцией, выполнить действие после сборки. Всё это выражается через кастомные команды и цели.

## Принципиальное различие двух команд

Это главное, что нужно понять, и источник частой путаницы:

**`add_custom_command`** описывает **как произвести файл** (или действие, привязанное к цели). Она сама по себе ничего не запускает — команда выполнится, только если её результат кому-то нужен. Управляется **зависимостями по файлам**.

**`add_custom_target`** создаёт **именованную цель**, которая выполняется **всегда** (по умолчанию считается «устаревшей»), независимо от файлов. Это точка в графе сборки, к которой можно обращаться по имени.

Проще говоря: `add_custom_command` — «рецепт для файла, готовится по требованию»; `add_custom_target` — «кнопка, которая жмётся всегда».

## `add_custom_command`: две формы

У команды две разные формы применения.

### Форма 1: генерация файла (OUTPUT)

Описывает, как создать выходной файл. Команда запустится, если этот файл нужен для сборки чего-то ещё **и** его нет или он устарел относительно входов.

```cmake
add_custom_command(
    OUTPUT generated.cpp                       # что производим
    COMMAND python ${CMAKE_SOURCE_DIR}/gen.py  # чем производим
            ${CMAKE_SOURCE_DIR}/schema.txt      # аргумент скрипта
            generated.cpp                        # аргумент — выходной файл
    DEPENDS ${CMAKE_SOURCE_DIR}/schema.txt      # от чего зависит вход
            ${CMAKE_SOURCE_DIR}/gen.py
    COMMENT "Генерация generated.cpp из schema.txt"
)

# Теперь используем сгенерированный файл в цели
add_executable(app main.cpp generated.cpp)
```

Ключевой механизм — **зависимости по файлам**: `DEPENDS` перечисляет входы. CMake перезапустит команду, только если `schema.txt` или `gen.py` изменились относительно `generated.cpp`. А связь «команда → цель» возникает потому, что `generated.cpp` указан в исходниках `app`: CMake видит, что для сборки `app` нужен `generated.cpp`, находит рецепт его создания и запускает при необходимости. Никакой явной привязки не нужно — она через имя выходного файла.

### Форма 2: действие вокруг цели (PRE_BUILD/PRE_LINK/POST_BUILD)

Привязывает команду к существующей цели — выполнить до или после её сборки:

```cmake
add_executable(app main.cpp)

add_custom_command(
    TARGET app POST_BUILD                       # после сборки app
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/config.ini
            $<TARGET_FILE_DIR:app>/config.ini    # рядом с бинарником
    COMMENT "Копирование config.ini к бинарнику"
)
```

Момент привязки: `POST_BUILD` — после линковки цели (самый частый), `PRE_LINK` — перед линковкой, `PRE_BUILD` — до всего (надёжно работает только с Visual Studio). Типичное применение POST_BUILD — копирование ресурсов, DLL, конфигов рядом с готовым исполняемым файлом.

Обрати внимание на `${CMAKE_COMMAND} -E copy` — это встроенные команды самого CMake, о них ниже.

## `add_custom_target`: всегда выполняемая цель

Создаёт именованную цель, не привязанную к конкретному выходному файлу. Выполняется каждый раз, когда её собирают.

```cmake
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Запуск всех тестов"
)
```

Вызов:

```
cmake --build build --target run_tests
```

Типичные применения кастомных целей: удобные команды-обёртки (форматирование кода, генерация документации, очистка), группировка нескольких кастомных команд, принудительное выполнение действия при каждой сборке.

Пример цели для форматирования кода:

```cmake
add_custom_target(format
    COMMAND clang-format -i ${ALL_SOURCE_FILES}
    COMMENT "Форматирование исходников через clang-format"
)
```

## Как связать: custom_command + custom_target

Частый паттерн: `add_custom_command(OUTPUT ...)` создаёт файл по требованию, но если этот файл не является исходником никакой цели, команда не запустится (её результат никому не нужен). Чтобы гарантированно её выполнять, к выходу привязывают кастомную цель через `DEPENDS`:

```cmake
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/version.h
    COMMAND ${CMAKE_COMMAND}
            -DOUT=${CMAKE_BINARY_DIR}/version.h
            -P ${CMAKE_SOURCE_DIR}/gen_version.cmake
    COMMENT "Генерация version.h"
)

# Цель, «требующая» этот файл, чтобы команда запускалась
add_custom_target(generate_version
    DEPENDS ${CMAKE_BINARY_DIR}/version.h
)

# Привязываем к основной цели, чтобы version.h генерировался перед её сборкой
add_executable(app main.cpp)
add_dependencies(app generate_version)
```

Здесь три звена: `add_custom_command` описывает рецепт файла, `add_custom_target` через `DEPENDS` делает файл «востребованным», а `add_dependencies(app generate_version)` гарантирует, что цель генерации отработает **до** сборки `app`. `add_dependencies` задаёт порядок между целями — `app` не начнёт собираться, пока не выполнится `generate_version`.

## Встроенные команды CMake: `${CMAKE_COMMAND} -E`

Важный кроссплатформенный приём. Вместо системных команд (`cp`, `copy`, `mkdir`, `rm`), которые различаются на Windows и Unix, используют встроенный «command mode» самого CMake — он работает одинаково везде:

```cmake
${CMAKE_COMMAND} -E copy src dst              # копировать файл
${CMAKE_COMMAND} -E copy_directory src dst    # копировать каталог
${CMAKE_COMMAND} -E make_directory dir        # создать каталог
${CMAKE_COMMAND} -E remove file               # удалить файл
${CMAKE_COMMAND} -E echo "текст"              # вывести текст
${CMAKE_COMMAND} -E create_symlink target link # символьная ссылка
${CMAKE_COMMAND} -E env VAR=value command      # запуск с переменной окружения
```

`${CMAKE_COMMAND}` — путь к исполняемому файлу CMake, а `-E` переключает его в режим утилиты. Полный список: `cmake -E help`. Всегда предпочитай их системным командам в кастомных командах — иначе сборка сломается при переносе на другую ОС.

## Полезные генераторные выражения для путей

В кастомных командах часто нужны пути к результату сборки, известные только на этапе генерации, — тут незаменимы генераторные выражения (тема, которую ты уже разбирал):

`$<TARGET_FILE:app>` — полный путь к бинарнику цели (с учётом имени, расширения, конфигурации). `$<TARGET_FILE_DIR:app>` — каталог, где лежит бинарник. Они корректно работают в multi-config генераторах, где путь зависит от Debug/Release, — обычная переменная тут не подошла бы.

## Практические правила

Различай две команды: `add_custom_command` — производит файл или действие вокруг цели (по требованию, через зависимости); `add_custom_target` — именованная цель, выполняемая всегда.

Для генерации файла, который потом компилируется, указывай его в исходниках цели (`add_executable`) — это и связывает рецепт с потреблением без лишних `add_dependencies`.

Если сгенерированный файл не является ничьим исходником, привязывай его к кастомной цели через `DEPENDS` и упорядочивай через `add_dependencies`, иначе команда не запустится.

Всегда используй `${CMAKE_COMMAND} -E` вместо системных `cp`/`copy`/`mkdir` — это единственный переносимый способ файловых операций в сборке.

Применяй `$<TARGET_FILE_DIR:...>` и подобные генераторные выражения для путей к результатам сборки — они верны в любой конфигурации.

Всегда указывай `COMMENT` — он выводится при сборке и делает понятным, какой шаг сейчас выполняется.

## Полный практический пример

Проект, где перед компиляцией генерируется заголовок с версией, а после сборки рядом с бинарником копируется конфиг.

### gen_version.cmake
скрипт-генератор, запускается в command mode
```cmake
# Пишет version.h с текущей датой  
string(TIMESTAMP BUILD_DATE "%Y-%m-%d")  
file(WRITE ${OUT}  
        "#pragma once\n"  
        "#define BUILD_DATE \"${BUILD_DATE}\"\n"  
        "#define BUILD_TYPE \"${BUILD_TYPE}\"\n"  
)
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 4.0.0)  
project(CustomDemo LANGUAGES CXX)  
  
# --- Генерация version.h перед сборкой ---  
add_custom_command(  
        OUTPUT ${CMAKE_BINARY_DIR}/generated/version.h  
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/generated  
        COMMAND ${CMAKE_COMMAND}  
        -DOUT=${CMAKE_BINARY_DIR}/generated/version.h  
        -DBUILD_TYPE=$<CONFIG>        -P ${CMAKE_SOURCE_DIR}/gen_version.cmake  
        DEPENDS ${CMAKE_SOURCE_DIR}/gen_version.cmake  
        COMMENT "Generation version.h"  
)  
  
add_executable(app main.cpp  
        # делает version.h востребованным  
        ${CMAKE_BINARY_DIR}/generated/version.h  
)  
target_include_directories(app PRIVATE ${CMAKE_BINARY_DIR}/generated)  
target_compile_features(app PUBLIC cxx_std_20)  
  
# --- Копирование конфига после сборки ---  
add_custom_command(  
        TARGET app POST_BUILD  
        COMMAND ${CMAKE_COMMAND} -E copy  
        ${CMAKE_SOURCE_DIR}/config.ini  
        $<TARGET_FILE_DIR:app>/config.ini        COMMENT "Copy config.ini -> bin"  
)
```

### main.cpp
```cpp
#include "version.h"  
#include <iostream>  
  
int main() {  
    std::cout << "Build: " << BUILD_TYPE << '\n';  
    std::cout << "Build: " << BUILD_DATE << '\n';  
}
```

При сборке: сначала генерируется `version.h` (потому что он в исходниках `app`), затем компилируется `app`, затем рядом с бинарником копируется `config.ini`. Изменение `gen_version.cmake` перегенерирует заголовок; при неизменных входах повторная генерация не происходит.