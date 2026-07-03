---
tags:
  - programming-language
  - cmake
---
[[programming languages/cmake/_|<=]]

CPack — встроенный в CMake инструмент, который берёт то, что описано командами `install`, и упаковывает в готовый **дистрибутив**: `.deb`/`.rpm` для Linux, `.zip`/`.tar.gz` как архивы, установщики для Windows (NSIS) и macOS (DragNDrop). Это завершает жизненный цикл — от исходников до пакета, который конечный пользователь ставит привычным способом.

## Связь с установкой

Ключевой принцип: CPack **не** описывает заново, что включать в пакет — он берёт это из твоих правил `install`. Всё, что ты настроил через `install(TARGETS ...)`, `install(DIRECTORY ...)` и т.д., автоматически попадёт в дистрибутив. Поэтому CPack — прямое продолжение темы установки: сначала корректно описываешь установку, потом CPack превращает её в пакет. Если установка настроена правильно, упаковка — это буквально несколько дополнительных строк.

## Минимальная настройка

CPack подключается модулем `include(CPack)`, а перед этим задаются переменные `CPACK_*`, описывающие пакет:

```cmake
cmake_minimum_required(VERSION 3.16)
project(MyApp VERSION 1.2.0 LANGUAGES CXX)

include(GNUInstallDirs)

add_executable(myapp src/main.cpp)

# Правила установки — их и упакует CPack
install(TARGETS myapp
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# --- Настройка CPack ---
set(CPACK_PACKAGE_NAME "MyApp")
set(CPACK_PACKAGE_VENDOR "Моя компания")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Краткое описание приложения")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_CONTACT "you@example.com")

# Какие генераторы пакетов использовать
set(CPACK_GENERATOR "TGZ;ZIP")

include(CPack)      # ВАЖНО: подключать последним, после всех CPACK_* переменных
```

Важная деталь: `include(CPack)` должен идти **после** установки всех переменных `CPACK_*`, потому что модуль считывает их значения в момент подключения. Переменные, заданные после `include(CPack)`, не подействуют.

## Запуск упаковки

После сборки CPack запускается из каталога сборки:

```
cmake -B build
cmake --build build
cd build && cpack           # или: cpack --config build/CPackConfig.cmake
```

Результат — файлы пакетов в каталоге сборки:

```
MyApp-1.2.0-Linux.tar.gz
MyApp-1.2.0-Linux.zip
```

Можно выбрать конкретный генератор при запуске, переопределив `CPACK_GENERATOR`:

```
cpack -G ZIP                # только ZIP
cpack -G DEB                # только .deb
```

## Генераторы пакетов

CPack поддерживает разные форматы через генераторы. Выбор задаётся в `CPACK_GENERATOR`:

Архивные (кроссплатформенные): `TGZ` (.tar.gz), `ZIP`, `TXZ` (.tar.xz) — просто архив с файлами, работает везде, не требует внешних инструментов.

Linux-пакеты: `DEB` (Debian/Ubuntu), `RPM` (Fedora/RHEL) — полноценные системные пакеты с зависимостями, устанавливаемые через `apt`/`dnf`.

Windows: `NSIS` (установщик .exe с мастером), `WIX` (.msi), `INNOSETUP`.

macOS: `DragNDrop` (.dmg), `productbuild` (.pkg), `Bundle`.

Часть генераторов требует установленных внешних инструментов (например, NSIS для Windows-установщика, `rpmbuild` для RPM), архивные — нет.

## Пример: пакеты для Linux (DEB и RPM)

У Linux-пакетов есть свои специфичные переменные — зависимости, секция, сопровождающий:

```cmake
set(CPACK_GENERATOR "DEB;RPM")

# Общие
set(CPACK_PACKAGE_NAME "myapp")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_CONTACT "you@example.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Утилита для примера")

# Специфично для DEB
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Имя Фамилия <you@example.com>")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.31)")   # зависимости пакета
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")

# Специфично для RPM
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_PACKAGE_REQUIRES "glibc >= 2.31")

include(CPack)
```

Обрати внимание на структуру имён: `CPACK_DEBIAN_*` действуют только для генератора DEB, `CPACK_RPM_*` — только для RPM. Это позволяет в одном файле настроить оба формата сразу, задав зависимости и метаданные для каждого по-своему.

Сборка пакетов:

```
cpack -G DEB
cpack -G RPM
```

Результат — `myapp-1.2.0-Linux.deb` и `myapp-1.2.0-Linux.rpm`, устанавливаемые штатными менеджерами:

```
sudo apt install ./myapp-1.2.0-Linux.deb
```

## Пример: установщик Windows (NSIS)

```cmake
set(CPACK_GENERATOR "NSIS")

set(CPACK_PACKAGE_NAME "MyApp")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "MyApp")    # каталог в Program Files

# Параметры установщика NSIS
set(CPACK_NSIS_DISPLAY_NAME "My Application")
set(CPACK_NSIS_PACKAGE_NAME "MyApp")
set(CPACK_NSIS_CONTACT "you@example.com")
set(CPACK_NSIS_MODIFY_PATH ON)                  # предложить добавить в PATH

include(CPack)
```

Это создаст `.exe`-установщик с графическим мастером (выбор каталога, ярлыки, деинсталлятор). Требует установленного NSIS на машине сборки.

## Лицензия и readme в пакете

Установщики (особенно NSIS, DragNDrop) часто показывают текст лицензии при установке. Его задают так:

```cmake
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set(CPACK_RESOURCE_FILE_README  "${CMAKE_CURRENT_SOURCE_DIR}/README.txt")
```

## Компоненты в пакете

Помнишь компоненты из темы установки (`COMPONENT runtime`, `COMPONENT development`)? CPack умеет собирать их в отдельные пакеты или в один пакет с выбором компонентов при установке. Включается это так:

```cmake
# при установке цели пометили компоненты
install(TARGETS mylib
    RUNTIME DESTINATION bin COMPONENT runtime
    ARCHIVE DESTINATION lib COMPONENT development
)

# CPack: собирать компоненты как отдельные пакеты
set(CPACK_COMPONENTS_ALL runtime development)
set(CPACK_DEB_COMPONENT_INSTALL ON)     # для DEB — по пакету на компонент

include(CPack)
```

Это позволяет разделить, например, пакет с самой программой (runtime) и пакет с заголовками и статическими библиотеками для разработчиков (development) — классическое деление `libfoo` и `libfoo-dev` в дистрибутивах Linux.

## Упаковка исходников

Кроме бинарных пакетов, CPack умеет паковать **исходники** проекта в архив для распространения — отдельным генератором:

```cmake
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_IGNORE_FILES
    "/build/"
    "/\\.git/"
    "\\.gitignore"
)
```

Запуск:

```
cpack --config CPackSourceConfig.cmake
```

`CPACK_SOURCE_IGNORE_FILES` — список регулярных выражений для исключения из исходного архива (каталог сборки, `.git` и прочий мусор). Результат — чистый архив с исходниками, годный для публикации релиза.

## CPack и пресеты

Как и остальные этапы, упаковку можно зафиксировать в пресетах (`packagePresets`, version 6+):

```json
{
  "version": 6,
  "packagePresets": [
    {
      "name": "release",
      "configurePreset": "release",
      "generators": ["TGZ", "DEB"]
    }
  ],
  "workflowPresets": [
    {
      "name": "dist",
      "steps": [
        { "type": "configure", "name": "release" },
        { "type": "build",     "name": "release" },
        { "type": "package",   "name": "release" }
      ]
    }
  ]
}
```

Тогда весь путь от исходников до пакета — одна команда:

```
cmake --workflow --preset dist
```

Это соединяет всё, что ты прошёл: конфигурацию, сборку, тесты (можно добавить шагом) и упаковку в единый воспроизводимый процесс для CI.

## Практические правила

Сначала настрой корректную установку (`install`), потом добавляй CPack — он упаковывает именно то, что описано в правилах установки, и без хорошей установки хорошего пакета не выйдет.

Подключай `include(CPack)` последним, после всех переменных `CPACK_*` — иначе они не будут прочитаны.

Начинай с архивных генераторов (`TGZ`, `ZIP`) — они работают везде без внешних инструментов; системные форматы (DEB, RPM, NSIS) добавляй, когда нужны, помня, что они требуют соответствующих утилит на машине сборки.

Используй `CPACK_<GENERATOR>_*` переменные для настроек конкретного формата (зависимости DEB, лицензия RPM, PATH в NSIS), задавая несколько форматов в одном файле.

Задействуй компоненты, если нужно разделить runtime и файлы для разработчиков в отдельные пакеты.

Фиксируй упаковку в workflow-пресетах для CI, чтобы релизный артефакт собирался одной командой и одинаково у всех.

# Пример

## Структура проекта

```
myapp/
├── CMakeLists.txt
├── CMakePresets.json
├── LICENSE.rtf              ← WIX требует лицензию в формате RTF
├── README.txt
└── src/
    └── main.cpp
```

Важная деталь сразу: генератор **WIX** показывает лицензию в мастере установки и требует её именно в формате **RTF** (не .txt), иначе установщик не соберётся. NSIS в этом смысле мягче (принимает .txt), но для MSI нужен RTF.

### src/main.cpp
```cpp
#include <iostream>  
  
int main() {  
    std::cout << "MyApp is working\n";  
}
```

### LICENSE.rtf
Минимальный валидный RTF (можно создать в WordPad, сохранив как RTF, либо вручную):
```rtf
{\rtf1\ansi
Лицензионное соглашение\par
\par
Copyright (c) 2026 Моя компания.\par
Разрешается использование на условиях лицензии MIT.\par
}
```

### CMakeLists.txt
```cpp
cmake_minimum_required(VERSION 4.0.0)  
project(MyApp VERSION 1.2.0 LANGUAGES CXX)  
  
include(GNUInstallDirs)  
  
add_executable(myapp src/main.cpp)  
target_compile_features(myapp PRIVATE cxx_std_20)  
  
# --- Установка: CPack упакует именно это ---  
install(TARGETS myapp  
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}  
)  
install(FILES README.txt DESTINATION .)  
  
# --- Общие метаданные пакета ---  
set(CPACK_PACKAGE_NAME "MyApp")  
set(CPACK_PACKAGE_VENDOR "My company")  
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})  
set(CPACK_PACKAGE_DESTINATION_SUMMARY "Application example with MSI-installer")  
set(CPACK_PACKAGE_CONTACT "you@example.com")  
# каталог в Program Files  
set(CPACK_PACKAGE_INSTALL_DIRECTORY "MyApp")  
  
# Лицензия (RTF!) и readme, показываемые в мастере  
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.rtf)  
set(CPACK_RESOURCE_FILE_README ${CMAKE_CURRENT_SOURCE_DIR}/README.txt)  
  
# --- Генератор MSI (WIX) ---  
set(CPACK_GENERATOR "WIX")  
  
# WIX-специфичные настройки  
# ФИКСИРОВАННЫЙ GUID  
set(CPACK_WIX_UPGRADE_GUID "8d10cc96-c923-44a6-91af-931f1775ef3f")  
# * = новый GUID на каждую сборку  
set(CPACK_WIX_PRODUCT_GUID "*")  
set(CPACK_WIX_PROPERTY_ARPHELPLINK "https://example.com/support")  
  
# последним, после всех CPACK_* переменных  
include(CPack)
```

Критически важный момент — `CPACK_WIX_UPGRADE_GUID`. Это фиксированный идентификатор продукта, по которому Windows понимает, что новая версия — это обновление уже установленного приложения, а не отдельная программа. Его нужно **сгенерировать один раз** (любым генератором GUID) и больше **никогда не менять** между версиями. Если менять — каждая версия будет ставиться параллельно как отдельный продукт, вместо того чтобы обновлять предыдущую. Сгенерировать GUID в PowerShell:

```powershell
[guid]::NewGuid()
```

Впиши полученное значение в `CPACK_WIX_UPGRADE_GUID` вместо примера выше.

### CMakePresets.json
```json
{  
    "version": 6,  
    "cmakeMinimumRequired": {  
        "major": 4,  
        "minor": 0,  
        "patch": 0  
    },  
    "configurePresets": [  
        {
			"name": "windows-release",  
            "displayName": "Windows Release (MSVC)",  
            "generator": "Visual Studio 17 2022",  
            "architecture": "x64",  
            "binaryDir": "${sourceDir}/build",  
            "cacheVariables": {  
                "CMAKE_BUILD_TYPE": "Release"  
            }  
        }
	],
	"buildPresets": [  
        {
			"name": "windows-release",  
            "configurePreset": "windows-release",  
            "configuration": "Release"  
        }  
    ],
	"packagePresets": [  
        {
			"name": "windows-msi",  
            "configurePreset": "windows-release",  
            "generators": ["WIX"]  
        }
	],
	"workflowPresets": [  
        {
			"name": "dist",  
            "steps": [  
                { "type": "configure", "name": "windows-release" },  
                { "type": "build",     "name": "windows-release" },  
                { "type": "package",   "name": "windows-msi" }  
            ]
		}
	]
}
```

Обрати внимание на пару деталей для Windows. `"architecture": "x64"` — для генератора Visual Studio архитектура задаётся отдельным полем, а не в имени генератора. `"configuration": "Release"` в build-пресете нужно потому, что Visual Studio — **multi-config** генератор: тип сборки выбирается на этапе сборки, а не конфигурации (это ты разбирал в теме конфигураций).

## Сборка

Одной командой весь путь — конфигурация, сборка, упаковка:

```powershell
cmake --workflow --preset dist
```

Либо по шагам:

```powershell
cmake --preset windows-release
cmake --build --preset windows-release
cpack --preset windows-msi
```

Результат в каталоге `build/`:

```
MyApp-1.2.0-win64.msi
```

Двойной клик по нему запустит стандартный мастер установки Windows с показом лицензии из `LICENSE.rtf`.

## Что нужно на машине сборки

Генератор WIX требует установленного **WiX Toolset** (v3) — CPack вызывает его утилиты (`candle`, `light`) для сборки MSI. Если WiX не установлен, `cpack` завершится с ошибкой про ненайденный `candle.exe`. Установить можно с сайта WiX Toolset или через пакетный менеджер:

```powershell
winget install WiXToolset.WiXToolset
```

Проверить текущую версию WiX стоит отдельно — экосистема WiX активно менялась (v3 против v4+ с другим синтаксисом вызова), и CPack-генератор WIX исторически рассчитан на WiX v3. Если у тебя возникнут проблемы со сборкой MSI, первым делом проверь, какая версия WiX установлена и совместима ли она с твоей версией CMake.

## Альтернатива: NSIS вместо WIX

Если MSI не обязателен, а нужен просто установщик `.exe`, генератор **NSIS** проще: он принимает лицензию в обычном `.txt` (не требует RTF) и не нуждается в GUID. Достаточно заменить в `CMakeLists.txt`:

```cmake
set(CPACK_GENERATOR "NSIS")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set(CPACK_NSIS_MODIFY_PATH ON)         # предложить добавить в PATH
```

и в пресете `"generators": ["NSIS"]`. NSIS-установщик даёт мастер с деинсталлятором и ярлыками; MSI выбирают, когда нужна интеграция с корпоративными системами развёртывания (Group Policy, SCCM), которые работают именно с `.msi`.

