---
tags:
  - build
---
[[programming languages/vcpkg/_|<=]]

# Создание собственных портов (ports)

Порт — это рецепт: как vcpkg должен получить исходники библиотеки, собрать их и разложить результат. Мы уже видели порты в каталоге `ports/` и упоминали их при разговоре о реестрах. Теперь разберём, как написать свой.

## Из чего состоит порт

Минимальный порт — это каталог с двумя файлами:

```
ports/
└── my-lib/
    ├── vcpkg.json        ← метаданные: имя, версия, зависимости, features
    └── portfile.cmake    ← инструкции: откуда взять и как собрать
```

- **`vcpkg.json`** — тот же формат манифеста, что мы разбирали, но в роли _описания порта_ (а не проекта). Здесь обязательны `name` и версия.
- **`portfile.cmake`** — императивный скрипт на CMake-языке, который выполняет vcpkg: скачать исходники, сконфигурировать, собрать, установить, прибрать за собой.

Опционально рядом лежат патчи (`.patch`-файлы) и вспомогательные `usage`-файлы.

## `vcpkg.json` порта

Для порта это описание самой библиотеки:

```json
{
  "name": "my-lib",
  "version": "1.2.0",
  "description": "A small geometry library",
  "homepage": "https://github.com/my-org/my-lib",
  "license": "MIT",
  "dependencies": [
    "fmt",
    {
      "name": "vcpkg-cmake",
      "host": true
    },
    {
      "name": "vcpkg-cmake-config",
      "host": true
    }
  ]
}
```

Обратите внимание на две служебные зависимости с `"host": true`:

- **`vcpkg-cmake`** — даёт функции `vcpkg_cmake_configure`/`vcpkg_cmake_build`/`vcpkg_cmake_install`.
- **`vcpkg-cmake-config`** — даёт `vcpkg_cmake_config_fixup` для корректной установки CMake-конфигов пакета.

Они нужны на _хосте_ (машине сборки), поэтому `host: true`. Это стандартные хелперы — почти любой современный CMake-порт их подключает.

## `portfile.cmake` — анатомия

Скрипт выполняется vcpkg в строго определённой среде с предопределёнными переменными (`SOURCE_PATH`, `CURRENT_PACKAGES_DIR`, `VCPKG_TARGET_TRIPLET` и т. д.). Типичная структура для CMake-библиотеки с GitHub:

```cmake
# 1. Получить исходники
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO my-org/my-lib
    REF v1.2.0
    SHA512 <512-битный-хеш-архива>
    HEAD_REF main
)

# 2. Сконфигурировать сборку (CMake)
vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DMYLIB_BUILD_TESTS=OFF
        -DMYLIB_BUILD_EXAMPLES=OFF
)

# 3. Собрать и установить
vcpkg_cmake_install()

# 4. Поправить расположение CMake-конфигов
vcpkg_cmake_config_fixup(PACKAGE_NAME my-lib CONFIG_PATH lib/cmake/my-lib)

# 5. Убрать дубли и лишнее
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")

# 6. Установить лицензию (обязательное правило vcpkg)
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
```

Разберём ключевые функции.

## `vcpkg_from_github` — получение исходников

Скачивает и распаковывает релиз/тег с GitHub, проверяя целостность по хешу.

```cmake
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH   # сюда запишется путь к распакованным исходникам
    REPO my-org/my-lib            # owner/repo
    REF v1.2.0                    # тег или коммит
    SHA512 abc123...              # контрольная сумма архива
    HEAD_REF main                 # ветка для режима --head
    PATCHES                       # опционально: патчи поверх исходников
        fix-build.patch
)
```

Что важно:

- **`SHA512`** обеспечивает воспроизводимость и безопасность — vcpkg проверит, что скачал ровно тот архив. Если хеш неизвестен, его подскажет сам vcpkg при первой (неудачной) попытке: запустите установку с заглушкой, и в ошибке будет правильный хеш, который надо вставить.
- **`REF`** — фиксированный тег/коммит, не плавающая ветка (для детерминизма).
- **`PATCHES`** — список патчей, применяемых сразу после распаковки (см. ниже).

Есть аналоги для других источников: `vcpkg_from_gitlab`, `vcpkg_from_bitbucket`, `vcpkg_from_git` (произвольный git), `vcpkg_download_distfile` (прямой URL архива).

## `vcpkg_cmake_configure` — конфигурация

Запускает конфигурацию CMake для исходников, прокидывая триплет, тип линковки и прочее автоматически.

```cmake
vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DMYLIB_BUILD_TESTS=OFF       # передаём флаги в CMake библиотеки
        -DMYLIB_WITH_FMT=ON
    OPTIONS_DEBUG
        -DMYLIB_VERBOSE=ON            # только для debug-конфигурации
    OPTIONS_RELEASE
        -DMYLIB_OPTIMIZE=ON           # только для release
)
```

Через `OPTIONS` вы управляете опциями сборки библиотеки — обычно отключаете тесты и примеры (они не нужны как артефакт и замедляют сборку).

### Связь features порта с опциями CMake

Если порт объявляет features, их обычно мапят на CMake-флаги через хелпер `vcpkg_check_features`:

```cmake
vcpkg_check_features(
    OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    FEATURES
        imaging   MYLIB_ENABLE_IMAGING
        network   MYLIB_ENABLE_NETWORK
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        ${FEATURE_OPTIONS}
)
```

Здесь feature `imaging` порта включает `-DMYLIB_ENABLE_IMAGING=ON`, если она активирована. Так замыкается история features из пункта про `vcpkg.json`: на уровне порта они превращаются в реальные флаги сборки.

## `vcpkg_cmake_install` — сборка и установка

```cmake
vcpkg_cmake_install()
```

Собирает (release и debug) и устанавливает результат в `CURRENT_PACKAGES_DIR` — staging-каталог, откуда потом всё попадёт в `installed/`. Обычно вызывается без аргументов.

## `vcpkg_cmake_config_fixup` — починка CMake-конфигов

Библиотеки ставят свои `*-config.cmake` в разные места (`lib/cmake/...`, `share/...`). vcpkg требует единообразия: конфиги должны лежать в `share/<пакет>/`, чтобы `find_package(... CONFIG)` их находил. Эта функция перемещает и правит их:

```cmake
vcpkg_cmake_config_fixup(
    PACKAGE_NAME my-lib
    CONFIG_PATH lib/cmake/my-lib
)
```

Без неё `find_package` в проекте может не найти пакет — частая причина «порт собрался, но не подключается».

## Обязательные правила установки

vcpkg проверяет результат порта и **ругается на нарушения**. Самые частые требования:

- **Лицензия обязательна.** Нужно установить файл лицензии:
    
    ```cmake
    vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
    ```
    
- **Нет `include` в debug.** Заголовки одинаковы для debug/release, дублировать нельзя:
    
    ```cmake
    file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
    ```
    
- **Нет `debug/share`.** Аналогично убирается.
- **Исполняемые инструменты** при наличии переносятся в `tools/<порт>/` хелпером `vcpkg_copy_tools`.

Эти правила обеспечивают чистую, предсказуемую структуру `installed/`.

## Патчи

Когда исходники библиотеки нужно слегка подправить (исправить сборку под конкретную платформу, отключить что-то несовместимое), используют патчи — обычные unified-diff файлы рядом с портом.

Создание патча:

```bash
# внутри распакованных исходников, под git
git diff > fix-build.patch
# положить файл в ports/my-lib/
```

Подключение в `portfile.cmake`:

```cmake
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO my-org/my-lib
    REF v1.2.0
    SHA512 ...
    PATCHES
        fix-build.patch
        disable-examples.patch
)
```

Патчи применяются по порядку сразу после распаковки. Это предпочтительнее правки исходников «на лету» командами `file()` — патч нагляден и версионируется.

## Локальное тестирование порта

Главный механизм — **overlay ports**: подсунуть каталог со своим портом, не добавляя его в реестр. (Мы упоминали overlay в пункте про реестры как «черновик/латку» — вот его прямое применение.)

```bash
# classic-style проверка установки порта
vcpkg install my-lib --overlay-ports=./my-ports

# или через переменную окружения
export VCPKG_OVERLAY_PORTS=./my-ports
```

В manifest-режиме overlay-порты подключаются через `VCPKG_OVERLAY_PORTS` в окружении или в `CMakePresets.json`:

```json
{
  "configurePresets": [
    {
      "name": "default",
      "toolchainFile": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake",
      "environment": {
        "VCPKG_OVERLAY_PORTS": "${sourceDir}/my-ports"
      }
    }
  ]
}
```

Цикл отладки порта:

1. Пишете `portfile.cmake` + `vcpkg.json` в `my-ports/my-lib/`.
2. `vcpkg install my-lib --overlay-ports=./my-ports`.
3. Смотрите ошибки в `buildtrees/my-lib/*.log` (как обсуждали в структуре каталогов).
4. Правите портфайл/патчи, повторяете.
5. Когда стабильно — переносите в git-реестр и регистрируете версию через `x-add-version`.

Полезные флаги при отладке:

- `--editable` — не «замораживать» исходники между запусками, удобно для итераций.
- `--no-binarycaching` — отключить бинарный кэш, чтобы пересборка происходила точно.

## Соберём порт для нашего примера

Допустим, `Point`/`Rect` из shapes-demo выросли в отдельную библиотеку `shapes-core` на GitHub (`my-org/shapes-core`), собираемую CMake и зависящую от `fmt`. Порт выглядел бы так.

`ports/shapes-core/vcpkg.json`:

```json
{
  "name": "shapes-core",
  "version": "1.0.0",
  "description": "Point and Rect geometry primitives",
  "license": "MIT",
  "dependencies": [
    "fmt",
    { "name": "vcpkg-cmake", "host": true },
    { "name": "vcpkg-cmake-config", "host": true }
  ]
}
```

`ports/shapes-core/portfile.cmake`:

```cmake
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO my-org/shapes-core
    REF v1.0.0
    SHA512 <вставить-реальный-хеш>
    HEAD_REF main
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DSHAPES_BUILD_TESTS=OFF
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(
    PACKAGE_NAME shapes-core
    CONFIG_PATH lib/cmake/shapes-core
)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
```

Тест локально:

```bash
vcpkg install shapes-core --overlay-ports=./ports
```

После успеха — публикация в приватный реестр (`x-add-version shapes-core`) и подключение в проекте через `vcpkg.json` + `vcpkg-configuration.json`, как в предыдущем пункте.

## Чек-лист корректного порта

- `vcpkg.json` с `name`, версией, зависимостями (включая host-хелперы `vcpkg-cmake*`).
- `portfile.cmake`: получение исходников с зафиксированным `REF` и `SHA512`.
- Отключены тесты/примеры через `OPTIONS`.
- Вызван `vcpkg_cmake_config_fixup` для конфигов.
- Удалён `debug/include` и `debug/share`.
- Установлена лицензия через `vcpkg_install_copyright`.
- Features (если есть) смаплены через `vcpkg_check_features`.
- Порт проверен через overlay-ports, логи чисты.

## Что запомнить

Порт = `vcpkg.json` (метаданные) + `portfile.cmake` (скрипт получения и сборки). Стандартный скелет CMake-порта: `vcpkg_from_github` (с `REF` и `SHA512`) → `vcpkg_cmake_configure` (с отключёнными тестами) → `vcpkg_cmake_install` → `vcpkg_cmake_config_fixup` → очистка debug-дублей → `vcpkg_install_copyright`. Правки исходников оформляются патчами, а не ручным редактированием. Отлаживают порт через overlay-ports, глядя в логи `buildtrees/`, после чего публикуют в реестр через `x-add-version`.

Дальше по плану — **продвинутые темы**: binary caching, asset caching, использование в CI/CD, кросс-компиляция, overlay ports/triplets. Перейдём?

---
---
---

## 9. Продвинутые темы

- Binary caching (кэширование собранных бинарников)
- Asset caching
- Использование в CI/CD (GitHub Actions, GitLab CI)
- Кросс-компиляция
- Overlay ports и overlay triplets

## 10. Практика

- Небольшой проект с 2–3 зависимостями в manifest mode
- Перевод существующего проекта на vcpkg
- Настройка binary cache в CI
- Создание и публикация собственного порта
