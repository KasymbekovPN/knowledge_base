---
tags:
  - build
---
[[programming languages/vcpkg/_|<=]]

## Binary caching (бинарное кэширование)

Самая ценная из продвинутых функций для практики.

### Проблема

vcpkg собирает библиотеки **из исходников**. Первая сборка boost или подобной тяжёлой библиотеки занимает минуты, а то и десятки минут. В CI, где окружение чистое при каждом запуске, это повторяется снова и снова — огромная потеря времени.

### Решение

**Binary cache** хранит уже собранные пакеты. Ключ кэша вычисляется из всего, что влияет на результат: версия порта, триплет, флаги, версии зависимостей, содержимое portfile. Если такой пакет уже собран — vcpkg берёт его из кэша вместо пересборки.

```
Нужен fmt@10.1.1, x64-linux, dynamic CRT
   → вычисляем хеш (ABI) всех влияющих факторов
   → есть в кэше? → распаковываем готовое (секунды)
   → нет?         → собираем и кладём в кэш на будущее
```

Этот хеш называют **ABI hash**: малейшее изменение (другая версия, другой флаг, другой триплет) даёт другой ключ — то есть кэш корректен и не отдаёт несовместимые бинарники.

### Где хранится кэш

Источники кэша (`binarysources`) бывают разные:

|Тип|Назначение|
|---|---|
|`files`|Локальный каталог (по умолчанию включён, `~/.cache/vcpkg` или `%LOCALAPPDATA%`)|
|`nuget`|NuGet-фид (часто Azure Artifacts, GitHub Packages) — командный кэш|
|`x-gha`|GitHub Actions cache — встроенный кэш CI|
|`x-azblob`|Azure Blob Storage|
|`x-aws` / `x-gcs`|S3 / Google Cloud Storage|
|`http`|Произвольный HTTP-сервер с GET/PUT|

### Настройка

Через переменную окружения `VCPKG_BINARY_SOURCES` или флаги. Примеры:

Локальный кэш (включён по умолчанию, можно указать явно):

```bash
export VCPKG_BINARY_SOURCES="clear;files,/path/to/cache,readwrite"
```

NuGet-фид для команды:

```bash
export VCPKG_BINARY_SOURCES="clear;nuget,https://my-feed/index.json,readwrite"
```

Синтаксис: источники разделяются `;`, у каждого указывается режим доступа `read` / `write` / `readwrite`. `clear` в начале сбрасывает дефолтные источники.

Типичный паттерн для CI: **читать и писать** общий кэш, чтобы первый прогон наполнил его, а последующие — переиспользовали.

## Asset caching (кэширование загрузок)

Связанная, но отдельная вещь. Binary cache хранит _собранные пакеты_; **asset cache** хранит _скачанные исходные архивы_ (то, что `vcpkg_from_github`/`vcpkg_download_distfile` тянут из интернета).

### Зачем

- **Надёжность**: если upstream-URL библиотеки умрёт или GitHub будет недоступен, сборка не сломается — архив берётся из вашего зеркала.
- **Скорость и изоляция**: загрузки идут с близкого зеркала, а не из интернета.
- **Закрытые окружения**: в средах без доступа в интернет asset cache (плюс binary cache) позволяет собирать полностью офлайн.

### Настройка

Через `X_VCPKG_ASSET_SOURCES`:

```bash
export X_VCPKG_ASSET_SOURCES="x-azurl,https://my-mirror/assets,,readwrite"
```

Доступны бэкенды вроде `x-azurl` (Azure/любой URL с SAS), и режим `x-block-origin` — запретить скачивание из оригинального источника, заставляя использовать только зеркало (полезно для воспроизводимости и закрытых сетей).

Asset cache использует тот самый `SHA512` из портов (из пункта про порты) как ключ — поэтому хеши в портфайлах важны не только для безопасности, но и для кэширования.

## Использование в CI/CD

Здесь сходятся все предыдущие темы. Принцип: vcpkg в CI должен быть **детерминированным** (через baseline из пункта про версионирование) и **быстрым** (через binary cache).

### Общая схема CI

1. Получить vcpkg (обычно как git submodule в проекте, либо клонировать на фиксированный коммит).
2. Настроить binary cache (и при необходимости asset cache).
3. Собрать проект — vcpkg сам поставит зависимости из манифеста, переиспользуя кэш.

### Vcpkg как submodule

Распространённый приём — добавить vcpkg субмодулем, чтобы зафиксировать его версию вместе с проектом (заодно это и есть baseline):

```bash
git submodule add https://github.com/microsoft/vcpkg.git
git -C vcpkg checkout <нужный-коммит>
git commit -am "Pin vcpkg"
```

### Пример: GitHub Actions

```yaml
name: build
on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest
    env:
      # GitHub Actions cache как binary cache для vcpkg
      VCPKG_BINARY_SOURCES: "clear;x-gha,readwrite"
    steps:
      - uses: actions/checkout@v4
        with:
          submodules: true        # подтягиваем vcpkg-субмодуль

      - name: Bootstrap vcpkg
        run: ./vcpkg/bootstrap-vcpkg.sh

      # Включаем экспорт токенов, нужных x-gha кэшу
      - name: Export GitHub Actions cache env
        uses: actions/github-script@v7
        with:
          script: |
            core.exportVariable('ACTIONS_CACHE_URL', process.env.ACTIONS_CACHE_URL || '');
            core.exportVariable('ACTIONS_RUNTIME_TOKEN', process.env.ACTIONS_RUNTIME_TOKEN || '');

      - name: Configure & build
        run: |
          cmake --preset default \
            -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake
          cmake --build build
```

Ключевые моменты:

- `VCPKG_BINARY_SOURCES: "clear;x-gha,readwrite"` — встроенный кэш GitHub Actions; первый прогон наполняет, дальнейшие переиспользуют собранные пакеты.
- `x-gha` требует прокинутых переменных `ACTIONS_CACHE_URL` и `ACTIONS_RUNTIME_TOKEN` — отсюда шаг с github-script.
- Зависимости из `vcpkg.json` ставятся автоматически на шаге конфигурации CMake (manifest mode).

### GitLab CI (эскиз)

```yaml
build:
  image: ubuntu:24.04
  variables:
    VCPKG_BINARY_SOURCES: "clear;files,$CI_PROJECT_DIR/.vcpkg-cache,readwrite"
  cache:
    paths:
      - .vcpkg-cache/      # сохраняем бинарный кэш между запусками job
  script:
    - ./vcpkg/bootstrap-vcpkg.sh
    - cmake --preset default
    - cmake --build build
```

Здесь binary cache хранится в каталоге проекта и сохраняется штатным механизмом кэширования GitLab между запусками.

## Кросс-компиляция

Сборка под платформу, отличную от хоста (например, на x64-Linux собрать под arm64 или под Android).

### Через триплеты

Целевая платформа задаётся триплетом (вспомним пункт про триплеты):

```bash
# на x64-хосте собираем под arm64
vcpkg install fmt:arm64-linux
```

Или в CMake:

```bash
cmake --preset default -DVCPKG_TARGET_TRIPLET=arm64-linux
```

### Host vs target

При кросс-компиляции появляется различие:

- **target triplet** — под что собираем итоговые библиотеки.
- **host triplet** — на чём работают инструменты времени сборки (генераторы кода и т. п.).

Поэтому в портах встречалось `"host": true` для зависимостей-инструментов — они собираются под host, а не под target. Host-триплет можно задать через `VCPKG_HOST_TRIPLET`, если автоопределение не подходит.

### Кастомный toolchain (для embedded/экзотики)

Для нестандартных целей в кастомном триплете указывают chainload-toolchain:

```cmake
set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_CMAKE_SYSTEM_NAME Linux)
set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/my-arm-toolchain.cmake")
```

`VCPKG_CHAINLOAD_TOOLCHAIN_FILE` подсовывает ваш CMake-toolchain (с указанием кросс-компилятора, sysroot и т. д.) — так vcpkg собирает зависимости тем же тулчейном, что и ваш проект.

## Overlay ports и overlay triplets

Мы их уже касались — соберём вместе как продвинутый механизм кастомизации без правки самого vcpkg.

### Overlay ports

Каталог со своими/пропатченными портами, имеющий приоритет над встроенным реестром:

```bash
export VCPKG_OVERLAY_PORTS=./my-ports
```

Применение: локальная разработка порта, временный патч публичной библиотеки, форк. (Для production-командной работы — полноценный реестр, как обсуждали.)

### Overlay triplets

Каталог со своими триплетами (из пункта про CRT — например, `x64-windows-static-md`):

```bash
export VCPKG_OVERLAY_TRIPLETS=./my-triplets
```

Применение: нестандартные комбинации линковки/CRT, особые флаги, embedded-цели.

Оба удобно фиксировать в `CMakePresets.json`, чтобы конфигурация ехала вместе с проектом:

```json
{
  "configurePresets": [
    {
      "name": "default",
      "toolchainFile": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake",
      "cacheVariables": {
        "VCPKG_OVERLAY_PORTS": "${sourceDir}/my-ports",
        "VCPKG_OVERLAY_TRIPLETS": "${sourceDir}/my-triplets"
      }
    }
  ]
}
```

## Как это связано с нашим примером

Для shapes-demo продвинутые темы дали бы:

- **binary cache** — `fmt` не пересобирается в каждом прогоне CI, экономия времени.
- **CI-пайплайн** — манифест + baseline обеспечивают одинаковую сборку у всех; `cmake --preset` ставит `fmt` автоматически.
- **кросс-компиляция** — тот же проект под arm64 простой сменой триплета.
- **overlay triplets** — собрать shapes-demo статически с нужной линковкой CRT (та самая комбинация из пункта про CRT).

## Что запомнить

Binary caching хранит собранные пакеты (ключ — ABI hash) и радикально ускоряет повторные/CI-сборки; asset caching хранит скачанные исходники для надёжности и офлайн-сборок. В CI vcpkg делают детерминированным через baseline/submodule и быстрым через binary cache (`x-gha` для GitHub Actions, `files`/`nuget` для других). Кросс-компиляция задаётся target-триплетом с разделением host/target и при необходимости chainload-toolchain. Overlay ports/triplets — способ кастомизировать порты и конфигурации без правки самого vcpkg.
