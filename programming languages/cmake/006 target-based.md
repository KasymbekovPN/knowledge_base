---
tags:
  - programming-language
  - cmake
---
[[programming languages/cmake/_|<=]]

Это ключевой сдвиг между «старым» и «современным» CMake. Старый подход настраивал свойства **глобально** — для всех целей в каталоге сразу. Современный (target-based) привязывает каждую настройку к **конкретной цели**, которая несёт её в себе и передаёт потребителям.

## Старые (глобальные) команды и их target-аналоги

|Глобальная (устаревшая)|Target-based (современная)|
|---|---|
|`include_directories()`|`target_include_directories()`|
|`add_definitions()`|`target_compile_definitions()`|
|`add_compile_options()`|`target_compile_options()`|
|`link_libraries()`|`target_link_libraries()`|
|`link_directories()`|`target_link_directories()`|

Общий признак старых команд — отсутствие имени цели в аргументах. Они действуют на всё, что объявлено после них в текущем каталоге и подкаталогах.

## В чём проблема глобального подхода

Рассмотрим типичный «старый» CMakeLists:

```cmake
# ❌ старый стиль
include_directories(libA/include)
include_directories(libB/include)
add_definitions(-DUSE_FEATURE_X)

add_executable(app1 app1.cpp)
add_executable(app2 app2.cpp)
add_library(core core.cpp)
```

Здесь **все** три цели — `app1`, `app2`, `core` — получают пути к заголовкам `libA` и `libB` и макрос `USE_FEATURE_X`, даже если им это не нужно. Возникают конкретные проблемы:

Загрязнение области видимости — цель видит заголовки, к которым не должна иметь доступа, что скрывает реальные зависимости и порождает случайные `#include`.

Невозможность разделить публичное и приватное — нельзя сказать «этот путь нужен только для сборки библиотеки, но не её потребителям».

Хрупкость при росте проекта — порядок команд начинает влиять на результат, настройки «протекают» между несвязанными модулями, и разобраться, откуда у цели взялся тот или иной флаг, становится тяжело.

Зависимости не передаются автоматически — слинковавшись с `core`, потребитель не узнает, какие заголовки и макросы нужны для использования `core`; их приходится прописывать вручную в каждом потребителе.

## Тот же проект в target-based стиле

```cmake
# ✅ современный стиль
add_library(core core.cpp)
target_include_directories(core PUBLIC include)        # потребители core унаследуют этот путь
target_compile_definitions(core PRIVATE CORE_INTERNAL) # только для сборки самой core

add_executable(app1 app1.cpp)
target_link_libraries(app1 PRIVATE core)               # app1 автоматически получит include от core

add_executable(app2 app2.cpp)
target_compile_definitions(app2 PRIVATE USE_FEATURE_X) # макрос только у app2, не у app1
```

Теперь каждая цель получает ровно то, что ей назначено. `app1` через линковку с `core` автоматически наследует публичный путь к заголовкам, а `app2` имеет свой макрос, не затрагивая `app1`.

## Главное преимущество — переносимость требований (usage requirements)

Суть target-based подхода в том, что цель сама описывает, **как её использовать**, и это описание передаётся по графу зависимостей через ключевые слова PUBLIC / PRIVATE / INTERFACE:

```cmake
add_library(json STATIC json.cpp)
target_include_directories(json PUBLIC include)
target_compile_features(json PUBLIC cxx_std_17)
target_compile_definitions(json PUBLIC JSON_USE_FAST_PARSER)
```

Любая цель, написавшая `target_link_libraries(myapp PRIVATE json)`, автоматически получит:

- путь `include` (для `#include <json.hpp>`),
- требование стандарта C++17,
- макрос `JSON_USE_FAST_PARSER`.

Ничего из этого не нужно прописывать в `myapp` вручную. Библиотека — самодостаточный «пакет требований», и это масштабируется на проекты из сотен целей.

## Наглядное сравнение распространения

```cmake
# Глобальный подход: настройка применяется ко всему ниже, направление — «сверху вниз»
include_directories(foo)        # → все последующие цели

# Target-подход: настройка привязана к цели, направление — «по графу зависимостей»
target_include_directories(mylib PUBLIC foo)
# → mylib + все, кто линкуется с mylib (транзитивно)
```

## Практические правила

Всегда предпочитайте команды с префиксом `target_`. Если в туториале или ответе видите `include_directories`/`add_definitions` без имени цели — это, скорее всего, устаревший материал.

Сознательно выбирайте видимость для каждой настройки: нужна ли она самой цели (PRIVATE/PUBLIC) и нужна ли потребителям (INTERFACE/PUBLIC).

Избегайте глобальных команд даже для «общих» настроек. Если несколько целей действительно должны разделять набор флагов, создайте для этого INTERFACE-библиотеку и линкуйте её:

```cmake
add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra>)

target_link_libraries(app1 PRIVATE project_warnings)
target_link_libraries(core PRIVATE project_warnings)
```

Это даёт общие настройки явно и контролируемо, без глобального «протекания».

**Вывод:** глобальные команды настраивают окружение, target-based команды настраивают цели. Современный CMake — это граф самодостаточных целей, где каждая знает свои требования и сама передаёт их тем, кто от неё зависит. Это делает проекты предсказуемыми, масштабируемыми и пригодными для повторного использования.
