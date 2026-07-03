---
tags:
  - programming-language
  - cmake
---
[[programming languages/cmake/_|<=]]

Кросс-компиляция — это сборка программы на одной платформе (host) для запуска на **другой** (target): например, собрать на x86-64 Linux бинарник для ARM, для микроконтроллера или для Android. Toolchain-файл — способ описать CMake, каким компилятором и окружением пользоваться для целевой платформы.

## Термины: host и target

Сначала разведём два понятия, иначе всё запутается:

**Host** — машина, на которой запущен CMake и происходит компиляция (твой рабочий компьютер).

**Target** — платформа, на которой будет **выполняться** результат (ARM-плата, микроконтроллер, телефон).

При обычной сборке host и target совпадают. При кросс-компиляции они разные, и CMake нужно объяснить, что не надо использовать системный компилятор host'а и не надо пытаться запускать собранные бинарники для проверки (они не запустятся на host'е).

## Что такое toolchain-файл

Toolchain-файл — это обычный `.cmake`-файл с набором переменных, описывающих целевую платформу: какой компилятор использовать, где лежат её библиотеки и заголовки, какая это система. Ты уже встречал toolchain-файл — vcpkg передаёт свой через `CMAKE_TOOLCHAIN_FILE`. Здесь тот же механизм, но файл описывает целевую платформу.

Ключевой принцип: toolchain-файл читается **очень рано**, ещё до `project()`, потому что `project()` запускает проверку компилятора — а компилятор как раз и задаётся в toolchain. Поэтому его нельзя подключить обычным `include()` внутри `CMakeLists.txt`; он передаётся снаружи при конфигурации:

```
cmake -B build -DCMAKE_TOOLCHAIN_FILE=/path/to/toolchain.cmake
```

## Основные переменные toolchain-файла

Минимальный набор, который описывает целевую платформу:

**`CMAKE_SYSTEM_NAME`** — имя целевой ОС. **Само присвоение этой переменной** переключает CMake в режим кросс-компиляции (устанавливает `CMAKE_CROSSCOMPILING` в TRUE). Значения: `Linux`, `Windows`, `Darwin`, `Android`, или `Generic` для «голого железа» без ОС (микроконтроллеры).

**`CMAKE_SYSTEM_PROCESSOR`** — архитектура целевого процессора (`arm`, `aarch64`, `x86_64` и т.д.).

**`CMAKE_C_COMPILER` / `CMAKE_CXX_COMPILER`** — путь к кросс-компилятору для целевой платформы.

**`CMAKE_FIND_ROOT_PATH`** — корень, где искать библиотеки и заголовки целевой платформы (sysroot).

**Переменные `CMAKE_FIND_ROOT_PATH_MODE_*`** — управляют тем, где `find_package`/`find_library`/`find_path` ищут: в системе host'а или только в целевом sysroot. Обычно программы (утилиты сборки) ищут на host'е, а библиотеки и заголовки — только в target.

## Пример: кросс-компиляция для ARM Linux

`toolchain-arm.cmake`:

```cmake
# Целевая система
set(CMAKE_SYSTEM_NAME Linux)              # активирует режим кросс-компиляции
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Кросс-компиляторы (префикс aarch64-linux-gnu- типичен для ARM64 Linux)
set(CMAKE_C_COMPILER   aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)

# Sysroot целевой платформы — где её библиотеки и заголовки
set(CMAKE_FIND_ROOT_PATH /usr/aarch64-linux-gnu)

# Программы ищем на host'е, всё остальное — только в target sysroot
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)   # не искать программы в sysroot
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)    # библиотеки только из sysroot
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)    # заголовки только из sysroot
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)    # пакеты только из sysroot
```

Использование:

```
cmake -B build-arm -DCMAKE_TOOLCHAIN_FILE=toolchain-arm.cmake
cmake --build build-arm
```

Результат — бинарник для ARM64, который **не запустится** на твоём x86-машине, но заработает на целевой ARM-плате.

Разбор режимов `FIND_ROOT_PATH_MODE`: `PROGRAM NEVER` означает «инструменты (компиляторы, генераторы кода) бери с host'а» — их нужно запускать во время сборки. `LIBRARY ONLY` / `INCLUDE ONLY` означают «библиотеки и заголовки бери **только** из целевого sysroot» — иначе CMake случайно подхватит библиотеки host'а, несовместимые с target. Это разделение — суть корректной кросс-сборки.

## Пример: «голое железо» (микроконтроллер)

Для встраиваемых систем без ОС ставят `CMAKE_SYSTEM_NAME Generic` и обычно отключают проверку компилятора, потому что CMake не может слинковать тестовую программу без стандартной среды выполнения:

```cmake
set(CMAKE_SYSTEM_NAME Generic)              # нет ОС
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER   arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)

# Проверка компилятора линковкой не сработает без runtime → проверяем как статическую библиотеку
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Флаги под конкретный МК (пример для Cortex-M4)
set(CMAKE_C_FLAGS_INIT   "-mcpu=cortex-m4 -mthumb")
set(CMAKE_CXX_FLAGS_INIT "-mcpu=cortex-m4 -mthumb")
```

`CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY` — важная деталь для embedded: по умолчанию CMake проверяет компилятор, собирая и **линкуя** тестовый исполняемый файл. На голом железе линковка требует стартап-кода и linker-скрипта, которых на этапе проверки ещё нет, поэтому проверку переводят в режим статической библиотеки (только компиляция, без линковки).

## Android

Для Android обычно не пишут toolchain-файл вручную — Android NDK поставляет готовый:

```
cmake -B build-android \
  -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
  -DANDROID_ABI=arm64-v8a \
  -DANDROID_PLATFORM=android-24
```

`ANDROID_ABI` задаёт архитектуру (`arm64-v8a`, `x86_64` и др.), `ANDROID_PLATFORM` — минимальную версию API. Это тот же механизм `CMAKE_TOOLCHAIN_FILE`, просто файл уже написан авторами NDK. Аналогично готовые toolchain-файлы дают Emscripten (WebAssembly), Yocto, производители embedded-SDK.

## Проверка режима кросс-компиляции в CMakeLists.txt

Внутри проекта можно реагировать на то, идёт ли кросс-сборка:

```cmake
if(CMAKE_CROSSCOMPILING)
    message(STATUS "Кросс-компиляция для ${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}")
    # например, не собирать инструменты, которые нужно запускать на host'е
else()
    message(STATUS "Нативная сборка")
endif()
```

`CMAKE_CROSSCOMPILING` автоматически становится TRUE, когда в toolchain задан `CMAKE_SYSTEM_NAME`. Это полезно, чтобы, например, пропустить сборку вспомогательных утилит, которые должны выполняться на host'е во время сборки (их для такого случая собирают отдельно нативным компилятором).

## Частая проблема: host-инструменты при кросс-сборке

Тонкий момент: иногда проект генерирует что-то с помощью собственной утилиты во время сборки (кодогенератор, компилятор ресурсов). При кросс-компиляции такая утилита собирается кросс-компилятором — и **не запустится на host'е**, где идёт сборка. Решение — собирать подобные host-инструменты отдельной нативной сборкой, а основную часть проекта кросс-компилировать. Это известная сложность больших кросс-проектов; для простых библиотек она не возникает.

## Toolchain-файл и пресеты

Как обсуждалось в теме пресетов, длинную команду с toolchain удобно зафиксировать в `CMakePresets.json`:

```json
{
  "version": 3,
  "configurePresets": [
    {
      "name": "arm",
      "displayName": "ARM64 Linux",
      "generator": "Ninja",
      "binaryDir": "${sourceDir}/build/arm",
      "toolchainFile": "${sourceDir}/cmake/toolchain-arm.cmake"
    }
  ]
}
```

Поле `toolchainFile` — специальное для пути к toolchain. Теперь кросс-сборка сводится к:

```
cmake --preset arm
```

Это особенно ценно, когда целей несколько (ARM, x86, embedded) — каждый пресет фиксирует свой toolchain, и переключение тривиально.

## Практические правила

Задавай `CMAKE_SYSTEM_NAME` в toolchain-файле — именно это включает режим кросс-компиляции; без него CMake считает сборку нативной.

Передавай toolchain-файл через `-DCMAKE_TOOLCHAIN_FILE` при конфигурации (или через `toolchainFile` в пресете), а не через `include()` — он должен читаться до `project()`.

Настраивай `CMAKE_FIND_ROOT_PATH` и режимы `FIND_ROOT_PATH_MODE_*`, чтобы библиотеки и заголовки брались из целевого sysroot, а инструменты — с host'а. Иначе подхватятся несовместимые библиотеки host'а.

Для «голого железа» ставь `CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY`, чтобы проверка компилятора не пыталась линковать.

Используй готовые toolchain-файлы (Android NDK, Emscripten, вендорские SDK), когда они есть, — не изобретай свой без необходимости.

Фиксируй toolchain в пресетах, если целевых платформ несколько, — это делает переключение между ними наглядным и воспроизводимым.
