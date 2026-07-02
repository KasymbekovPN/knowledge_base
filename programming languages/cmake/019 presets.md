---
tags:
  - programming-language
  - cmake
---
[[programming languages/cmake/_|<=]]

`CMakePresets.json` — файл, который фиксирует параметры сборки (пути, генератор, toolchain, опции, тип конфигурации) в одном месте, чтобы не передавать длинные команды с `-D` вручную каждый раз и чтобы вся команда и CI собирали проект **одинаково**.

## Проблема, которую решают пресеты

По ходу плана команды конфигурации разрастались:

```
cmake -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
  -DCALC_BUILD_TESTS=ON \
  -DCMAKE_INSTALL_PREFIX=install
```

Это трудно запомнить, легко ошибиться, и у каждого разработчика команда своя. Пресеты записывают всё это один раз в `CMakePresets.json`, после чего конфигурация сводится к:

```
cmake --preset default
```

## Расположение и структура

Файл `CMakePresets.json` кладётся в **корень проекта** (рядом с верхним `CMakeLists.txt`) и обычно коммитится в репозиторий — это его смысл, общие настройки для всех. Личные переопределения идут в `CMakeUserPresets.json` (его добавляют в `.gitignore`).

Файл состоит из нескольких видов пресетов, главные — **configure** (этап конфигурации) и **build** (этап сборки). Есть также test, package и workflow.

## Минимальный пример

`CMakePresets.json`:

```json
{
  "version": 3,
  "cmakeMinimumRequired": {
    "major": 3,
    "minor": 21,
    "patch": 0
  },
  "configurePresets": [
    {
      "name": "default",
      "displayName": "Сборка по умолчанию (Ninja, Release)",
      "generator": "Ninja",
      "binaryDir": "${sourceDir}/build",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Release"
      }
    }
  ]
}
```

Использование:

```
cmake --preset default        # конфигурация
cmake --build build           # сборка
```

Разберём ключевые поля configure-пресета:

`name` — идентификатор, по которому вызывают пресет (`--preset default`).

`generator` — генератор сборки (Ninja, «Unix Makefiles», «Visual Studio 17 2022»).

`binaryDir` — каталог сборки. `${sourceDir}` — встроенная переменная, указывающая на корень проекта, так что путь не завязан на то, откуда запущен CMake.

`cacheVariables` — набор кэш-переменных, тот самый эквивалент флагов `-D`. Сюда идёт всё, что раньше передавалось через `-DИМЯ=значение`.

## Пример с несколькими пресетами и наследованием

Реальная сила — в наборе пресетов под разные сценарии (Debug/Release, разные компиляторы), где общие настройки выносятся в базовый пресет через `inherits`.

```json
{
  "version": 3,
  "configurePresets": [
    {
      "name": "base",
      "hidden": true,
      "generator": "Ninja",
      "binaryDir": "${sourceDir}/build/${presetName}",
      "cacheVariables": {
        "CALC_BUILD_TESTS": "ON"
      }
    },
    {
      "name": "debug",
      "displayName": "Debug",
      "inherits": "base",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug"
      }
    },
    {
      "name": "release",
      "displayName": "Release",
      "inherits": "base",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Release"
      }
    }
  ]
}
```

Ключевые приёмы здесь:

`"hidden": true` — пресет `base` не показывается в списке и не вызывается напрямую; он существует только как основа для наследования. Это чистый способ вынести общие настройки.

`inherits` — пресеты `debug` и `release` наследуют всё из `base` (генератор, каталог, опцию тестов) и добавляют/переопределяют своё (тип сборки). Не нужно дублировать общие поля.

`binaryDir` с `${presetName}` — каждый пресет получает свой каталог сборки (`build/debug`, `build/release`), так что Debug и Release не конфликтуют и живут параллельно.

Использование:

```
cmake --preset debug
cmake --build build/debug

cmake --preset release
cmake --build build/release
```

Посмотреть доступные пресеты:

```
cmake --list-presets
```

## Build-пресеты

Configure-пресеты настраивают конфигурацию, build-пресеты — сборку. Они привязываются к configure-пресету через `configurePreset`:

```json
{
  "version": 3,
  "configurePresets": [ ... ],
  "buildPresets": [
    {
      "name": "release",
      "configurePreset": "release",
      "jobs": 8
    },
    {
      "name": "release-tests",
      "configurePreset": "release",
      "targets": ["calc_test"]
    }
  ]
}
```

Теперь сборка тоже через пресет:

```
cmake --preset release          # конфигурация
cmake --build --preset release  # сборка (8 потоков)
```

Поля: `jobs` — параллелизм (аналог `-j 8`), `targets` — какие цели собирать, `configuration` — тип сборки для multi-config генераторов.

## Test-пресеты

Аналогично для CTest, привязываются к configure-пресету:

```json
{
  "testPresets": [
    {
      "name": "release",
      "configurePreset": "release",
      "output": { "outputOnFailure": true },
      "execution": { "jobs": 4 }
    }
  ]
}
```

Запуск тестов:

```
ctest --preset release
```

`outputOnFailure: true` — тот самый ключ из темы тестирования, теперь зафиксированный в пресете, чтобы не вспоминать его каждый раз.

## Пример с toolchain vcpkg

Собирая воедино темы плана — пресет с интеграцией vcpkg, чтобы не передавать длинный путь к toolchain вручную:

```json
{
  "version": 3,
  "configurePresets": [
    {
      "name": "vcpkg",
      "displayName": "Release + vcpkg",
      "generator": "Ninja",
      "binaryDir": "${sourceDir}/build",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Release",
        "CMAKE_TOOLCHAIN_FILE": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      }
    }
  ]
}
```

`$env{VCPKG_ROOT}` — обращение к переменной окружения. Смысл: путь к vcpkg берётся из `VCPKG_ROOT`, заданной в системе, а не хардкодится в файле, — так пресет одинаково работает у всех, у кого настроена эта переменная. Теперь вся сложная конфигурация из начала ответа сводится к:

```
cmake --preset vcpkg
```

## Переменные и условия

Пресеты поддерживают несколько видов подстановок:

`${sourceDir}` — корень проекта. `${presetName}` — имя текущего пресета. `${sourceParentDir}` — родитель корня. `$env{ИМЯ}` — переменная окружения. `$penv{ИМЯ}` — переменная окружения на момент запуска (для сложных случаев).

Есть и условия (`condition`), позволяющие включать пресет только на определённой платформе — например, отдельные пресеты для Windows и Linux. Это удобно для кроссплатформенных проектов, но для начала достаточно базовых пресетов без условий.

## Workflow-пресеты (версия 6+)

Начиная с version 6, есть workflow-пресеты, объединяющие конфигурацию, сборку и тесты в одну команду:

```json
{
  "version": 6,
  "workflowPresets": [
    {
      "name": "ci",
      "steps": [
        { "type": "configure", "name": "release" },
        { "type": "build",     "name": "release" },
        { "type": "test",      "name": "release" }
      ]
    }
  ]
}
```

Запуск всей цепочки одной командой:

```
cmake --workflow --preset ci
```

Это особенно ценно для CI: одна команда делает всё, и локально можно воспроизвести ровно то, что происходит на сервере сборки.

## Интеграция с IDE

Важное практическое преимущество: пресеты понимают современные IDE и редакторы — VS Code (через расширение CMake Tools), Visual Studio, CLion, Qt Creator. Они читают `CMakePresets.json` и показывают пресеты в выпадающем списке, так что переключение между Debug и Release становится выбором из меню, а не правкой команд. Это одна из главных причин популярности пресетов в командах.

## `CMakePresets.json` vs `CMakeUserPresets.json`

Разделение по назначению: `CMakePresets.json` — общие пресеты проекта, коммитятся в репозиторий, одинаковы для всех. `CMakeUserPresets.json` — личные пресеты конкретного разработчика (свои пути, свой компилятор, локальные эксперименты), добавляются в `.gitignore` и не попадают в репозиторий. Пользовательский файл может наследовать пресеты из основного через `inherits`, переопределяя нужное.

## Практические правила

Коммить `CMakePresets.json` в репозиторий — в этом его смысл: единый воспроизводимый способ сборки для всей команды и CI.

Выноси общие настройки в скрытый (`"hidden": true`) базовый пресет и наследуй от него через `inherits` — избегаешь дублирования.

Давай каждому configure-пресету свой `binaryDir` с `${presetName}`, чтобы разные конфигурации собирались в отдельных каталогах и не конфликтовали.

Личные и машинно-зависимые настройки (абсолютные пути, локальные toolchain) держи в `CMakeUserPresets.json`, а не в общем файле.

Для CI используй workflow-пресеты (version 6+) или последовательность `--preset` команд — чтобы сервер собирал ровно то же, что разработчик локально.

Обращайся к машинным путям через `$env{...}` вместо хардкода — так один пресет работает у всех.
