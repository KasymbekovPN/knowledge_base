---
tags:
  - build
---
[[programming languages/vcpkg/_|<=]]

# Установка vcpkg

Установка состоит из двух шагов: клонировать репозиторий и запустить bootstrap-скрипт, который соберёт сам исполняемый файл vcpkg.

## Шаг 0. Предварительные требования

Перед установкой нужны:

- **Git** — для клонирования.
- **Компилятор C++** — bootstrap собирает vcpkg из исходников, поэтому нужен рабочий тулчейн.
    - Windows: Visual Studio 2019/2022 с компонентом «Desktop development with C++» (или Build Tools).
    - Linux: `gcc`/`g++`, плюс обычно `build-essential`, `curl`, `zip`, `unzip`, `tar`, `pkg-config`.
    - macOS: Xcode Command Line Tools (`xcode-select --install`).
- **CMake** (желательно свежий, хотя vcpkg тянет собственный при необходимости).

## Шаг 1. Клонирование репозитория

Выбираете каталог, где будет жить vcpkg. Часто кладут в домашнюю папку или корень диска — путь не должен быть слишком длинным и без пробелов/кириллицы (особенно важно на Windows).

```bash
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
```

После этого у вас есть исходники vcpkg, но самого исполняемого файла ещё нет — его создаёт bootstrap.

## Шаг 2. Запуск bootstrap-vcpkg

Скрипт компилирует исполняемый файл `vcpkg` из исходников.

**Windows (PowerShell или cmd):**

```bat
.\bootstrap-vcpkg.bat
```

**Linux / macOS:**

```bash
./bootstrap-vcpkg.sh
```

После успешного выполнения в корне появится исполняемый файл:

- `vcpkg.exe` на Windows,
- `vcpkg` на Linux/macOS.

Проверка, что всё работает:

```bash
./vcpkg version
```

## Полезные флаги bootstrap

- `-disableMetrics` (Windows) / `./bootstrap-vcpkg.sh -disableMetrics` — отключает отправку анонимной телеметрии. Многие включают сразу.
- На Linux/macOS, если хотите использовать системный компилятор вместо подтягиваемого — обычно ничего дополнительно не нужно, скрипт сам находит тулчейн.

Отключить телеметрию можно и иначе — создав пустой файл `vcpkg.disable-metrics` в корне vcpkg.

## Шаг 3. Переменная окружения VCPKG_ROOT

Это не обязательная часть установки, но крайне желательная: многие toolchain-настройки и пресеты ссылаются на `$VCPKG_ROOT`. Удобно задать её на корень vcpkg.

**Linux / macOS** (добавить в `~/.bashrc` или `~/.zshrc`):

```bash
export VCPKG_ROOT=/path/to/vcpkg
export PATH=$VCPKG_ROOT:$PATH
```

**Windows (PowerShell, для текущего пользователя):**

```powershell
[Environment]::SetEnvironmentVariable("VCPKG_ROOT", "C:\vcpkg", "User")
```

И при желании добавить `C:\vcpkg` в `Path`, чтобы вызывать `vcpkg` из любого места.

После этого перезапустите терминал, чтобы переменные подхватились.

## Шаг 4. (Опционально) Интеграция с системой

Для classic-режима и Visual Studio можно выполнить глобальную интеграцию, чтобы MSBuild/VS автоматически видели установленные пакеты:

```bash
./vcpkg integrate install
```

В manifest-режиме (рекомендуемом) это обычно не требуется — там всё подключается через toolchain-файл проекта.

## Что в итоге

После этих шагов у вас:

1. Склонированный репозиторий vcpkg.
2. Собранный исполняемый файл vcpkg.
3. Заданная переменная `VCPKG_ROOT`, на которую будут ссылаться CMake-пресеты и toolchain.
