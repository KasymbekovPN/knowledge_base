---
tags:
  - build
---
[[programming languages/vcpkg/_|<=]]

# `vcpkg.json` и features подробно

Файл `vcpkg.json` — это манифест проекта: декларативное описание того, что проекту нужно. Разберём сначала все поля манифеста, затем подробно — features, поскольку это самая нетривиальная часть.

## Полная структура `vcpkg.json`

```json
{
  "name": "shapes-demo",
  "version": "0.1.0",
  "description": "Demo project with shapes and fmt",
  "homepage": "https://example.com/shapes-demo",
  "license": "MIT",
  "supports": "!uwp",
  "dependencies": [
    "fmt",
    {
      "name": "curl",
      "features": ["ssl"],
      "default-features": false,
      "version>=": "8.0.0"
    }
  ],
  "features": {
    "imaging": {
      "description": "Image export support",
      "dependencies": ["libpng", "libjpeg-turbo"]
    },
    "tests": {
      "description": "Build with test framework",
      "dependencies": ["catch2"]
    }
  },
  "default-features": ["imaging"],
  "overrides": [
    { "name": "fmt", "version": "10.1.1" }
  ],
  "builtin-baseline": "<git-хеш-коммита-vcpkg>"
}
```

Не все поля обязательны — минимальный валидный манифест может быть просто `{ "dependencies": ["fmt"] }`. Разберём поля по группам.

## Поля-метаданные

|Поле|Назначение|Обязательно|
|---|---|---|
|`name`|Имя проекта/порта. Только строчные буквы, цифры, дефисы|Да, если есть `version*` или проект публикуется как порт|
|`version` / `version-semver` / `version-date` / `version-string`|Версия проекта (разные схемы версионирования, см. ниже)|Нет (для приложения), да для порта|
|`description`|Человекочитаемое описание|Нет|
|`homepage`, `documentation`, `license`|Метаданные о проекте|Нет|
|`supports`|Выражение поддерживаемых платформ (`"windows & !static"`)|Нет|

### Четыре схемы поля версии

vcpkg различает схему версии по имени поля — это влияет на правила сравнения:

- `version` — числовая схема с точками: `1.2.3`. Самая частая.
- `version-semver` — строгий semver: `1.2.3-alpha.1`.
- `version-date` — дата: `2024-01-15` (для библиотек, релизящихся по датам).
- `version-string` — произвольная строка без упорядочивания (например, `"vendor-custom"`); сравнение версий тогда невозможно.

Для приложения (не публикуемого как порт) поле версии в целом опционально и нужно скорее для порядка.

## `dependencies` — зависимости

Главное поле. Каждый элемент — либо строка (просто имя порта), либо объект с деталями.

### Краткая форма

```json
"dependencies": [ "fmt", "zlib", "spdlog" ]
```

### Развёрнутая форма (объект)

```json
"dependencies": [
  {
    "name": "curl",
    "features": ["ssl", "http2"],
    "default-features": false,
    "version>=": "8.0.0",
    "platform": "!windows"
  }
]
```

Поля объекта зависимости:

- **`name`** — имя порта (обязательно).
- **`features`** — какие опциональные компоненты этого порта включить (см. раздел про features ниже).
- **`default-features`** — включать ли стандартный набор features порта. `false` отключает их (берём только то, что явно перечислили). По умолчанию `true`.
- **`version>=`** — минимальная требуемая версия порта. vcpkg возьмёт эту или новее (в рамках baseline).
- **`platform`** — условие, при котором зависимость нужна. Например, `"windows"` — ставить только на Windows; `"!osx"` — везде кроме macOS. Поддерживает `&`, `|`, `!`, скобки.
- **`host`** — зависимость для хост-машины сборки (нужна при кросс-компиляции для инструментов времени сборки).

### Пример условных зависимостей

```json
"dependencies": [
  "fmt",
  { "name": "wil", "platform": "windows" },
  { "name": "libsystemd", "platform": "linux" }
]
```

`wil` поставится только на Windows, `libsystemd` — только на Linux. Удобно для кросс-платформенных проектов.

## Features — опциональные возможности

Теперь главная тема. **Feature** — это именованный набор дополнительных возможностей и зависимостей, который можно включать или выключать. Идея: не тащить в проект всё подряд, а подключать только нужные части.

Features бывают в двух ролях, и их легко спутать:

1. **Features, которые ваш проект объявляет о себе** (поле `"features"` в вашем манифесте).
2. **Features зависимостей, которые вы включаете** (поле `"features"` внутри элемента `dependencies`).

Разберём обе.

### Роль 1. Включение features у зависимости

У многих портов есть опциональные компоненты. Классический пример — `curl`: его можно собрать с поддержкой SSL, HTTP/2, разными бэкендами и без них. Эти варианты оформлены как features порта.

```json
"dependencies": [
  {
    "name": "curl",
    "default-features": false,
    "features": ["ssl", "http2"]
  }
]
```

Что здесь происходит:

- `"features": ["ssl", "http2"]` — явно просим собрать curl с поддержкой SSL и HTTP/2.
- `"default-features": false` — отключаем стандартный набор features curl, чтобы получить минимальную сборку плюс только то, что мы указали. Без этого к нашим двум добавился бы дефолтный набор порта.

Узнать, какие features есть у порта, можно через поиск/inspection:

```bash
vcpkg search curl          # features показываются в скобках
vcpkg depend-info curl     # дерево зависимостей с учётом features
```

Зачем это нужно:

- **Меньше время сборки и размер**: не собираем неиспользуемые куски.
- **Контроль зависимостей**: каждая feature может тянуть свои библиотеки; отключив лишнее, не тащим лишние транзитивные зависимости.
- **Контроль лицензий/совместимости**: можно отказаться от компонента с нежелательной лицензией.

### Роль 2. Объявление собственных features проекта

Ваш проект тоже может определять features — в поле верхнего уровня `"features"`. Это позволяет делать **сам ваш проект конфигурируемым**: разные сборки тянут разные наборы зависимостей.

```json
{
  "name": "shapes-demo",
  "version": "0.1.0",
  "dependencies": [ "fmt" ],
  "features": {
    "imaging": {
      "description": "Export shapes to PNG/JPEG",
      "dependencies": ["libpng", "libjpeg-turbo"]
    },
    "tests": {
      "description": "Unit tests",
      "dependencies": ["catch2"]
    }
  }
}
```

Здесь:

- `fmt` — базовая (всегда нужная) зависимость, она в `dependencies`.
- `imaging` — опциональная возможность: если включена, добавляются `libpng` и `libjpeg-turbo`.
- `tests` — опциональная: добавляет тестовый фреймворк `catch2`.

По умолчанию features проекта **выключены**, если их не активировать.

### Как активировать features проекта при сборке

Несколько способов.

**Через CMake-переменную `VCPKG_MANIFEST_FEATURES`:**

```bash
cmake --preset default -DVCPKG_MANIFEST_FEATURES="imaging;tests"
```

**Через `CMakePresets.json`:**

```json
{
  "version": 3,
  "configurePresets": [
    {
      "name": "full",
      "generator": "Ninja",
      "binaryDir": "${sourceDir}/build",
      "toolchainFile": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake",
      "cacheVariables": {
        "VCPKG_MANIFEST_FEATURES": "imaging;tests"
      }
    }
  ]
}
```

**Через CLI vcpkg (classic-style вызов в каталоге с манифестом):**

```bash
vcpkg install --x-feature=imaging --x-feature=tests
```

### `default-features` на уровне проекта

Поле верхнего уровня `"default-features"` задаёт, какие из ваших features включены по умолчанию (когда никто явно ничего не выбрал):

```json
"features": {
  "imaging": { "description": "...", "dependencies": ["libpng"] },
  "tests":   { "description": "...", "dependencies": ["catch2"] }
},
"default-features": ["imaging"]
```

Теперь `imaging` активна по умолчанию, а `tests` — нет (нужно включать явно). Это типичный приём: «полезные по умолчанию» возможности включены, «тяжёлые/служебные» (как тесты) — по запросу.

### Features могут зависеть друг от друга

Внутри feature в `dependencies` можно ссылаться на другие features того же проекта или включать конкретные features зависимостей:

```json
"features": {
  "network": {
    "description": "Networking support",
    "dependencies": [
      { "name": "curl", "features": ["ssl"] }
    ]
  },
  "full": {
    "description": "Everything",
    "dependencies": [
      { "name": "shapes-demo", "features": ["network", "imaging"] }
    ]
  }
}
```

Здесь feature `full` через ссылку на собственное имя проекта включает features `network` и `imaging`. Это паттерн «зонтичной» feature, активирующей сразу несколько.

## `overrides` и `builtin-baseline` (кратко — детальнее в пункте про версионирование)

- **`builtin-baseline`** — git-хеш коммита репозитория vcpkg, фиксирующий «снимок» реестра. Все версии берутся из этого снимка → воспроизводимость.
- **`overrides`** — принудительно зафиксировать конкретную версию пакета, игнорируя ограничения остальных:

```json
"overrides": [
  { "name": "fmt", "version": "10.1.1" }
]
```

Эти два поля разберём подробно в следующем пункте плана.

## Применим к нашему примеру

Расширим `vcpkg.json` из проекта shapes-demo, добавив опциональную поддержку вывода в изображение и тестов:

```json
{
  "name": "shapes-demo",
  "version": "0.1.0",
  "description": "Shapes (Point, Rect) demo using fmt",
  "dependencies": [
    "fmt"
  ],
  "features": {
    "imaging": {
      "description": "Render shapes to PNG",
      "dependencies": ["libpng"]
    },
    "tests": {
      "description": "Unit tests with Catch2",
      "dependencies": ["catch2"]
    }
  },
  "default-features": []
}
```

И в `CMakeLists.txt` соответствующие части подключаются по условию:

```cmake
find_package(fmt CONFIG REQUIRED)

add_executable(shapes-demo src/main.cpp src/shapes.cpp)
target_include_directories(shapes-demo PRIVATE include)
target_link_libraries(shapes-demo PRIVATE fmt::fmt)

# Подключаем libpng только если активирована feature imaging.
# Простой способ — проверять, найден ли пакет.
find_package(PNG CONFIG QUIET)
if(PNG_FOUND)
    target_link_libraries(shapes-demo PRIVATE PNG::PNG)
    target_compile_definitions(shapes-demo PRIVATE SHAPES_WITH_IMAGING)
endif()
```

Сборка с включённой feature:

```bash
cmake --preset default -DVCPKG_MANIFEST_FEATURES="imaging"
cmake --build build
```

Теперь `libpng` ставится только когда вы явно запросили `imaging`; обычная сборка остаётся лёгкой и тянет только `fmt`.

## Частые ошибки и нюансы

- **Путаница двух `features`**: поле верхнего уровня `"features"` определяет возможности _вашего_ проекта; поле `"features"` _внутри_ элемента `dependencies` включает возможности _чужого_ порта. Это разные вещи.
- **Забыли `default-features: false`** при включении конкретных features зависимости — тогда к вашему выбору добавится дефолтный набор порта, и сборка может тянуть лишнее.
- **`name` обязателен**, если в манифесте есть `version`/`features`/`builtin-baseline`. Минимальный манифест без них может обойтись без `name`.
- **Имена только в нижнем регистре** с дефисами — `My_Project` невалидно.
- **Активация features проекта идёт не из самого `vcpkg.json`**, а при конфигурации (через `VCPKG_MANIFEST_FEATURES` или `--x-feature`). В манифесте вы их только _определяете_ и задаёте `default-features`.

## Что запомнить

`vcpkg.json` — декларативный манифест проекта: метаданные, `dependencies`, собственные `features`, ограничения версий. Features — механизм опциональности: вы либо **включаете** features у зависимости (минимизируя сборку чужого порта), либо **объявляете** свои features, делая собственный проект конфигурируемым. По умолчанию features выключены, активируются через `VCPKG_MANIFEST_FEATURES`/`default-features`. Главное — не путать «features моего проекта» и «features зависимости».
