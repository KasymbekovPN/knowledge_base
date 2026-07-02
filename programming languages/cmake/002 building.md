---
tags:
  - programming-language
  - cmake
---
[[programming languages/cmake/_|<=]]

Сборка проекта в CMake — это всегда два отдельных этапа: сначала **конфигурация и генерация**, затем собственно **сборка**.

**Этап 1: `cmake -B build`**

Это этап конфигурации и генерации. CMake читает `CMakeLists.txt`, проверяет компиляторы, разрешает зависимости и генерирует файлы для конкретной системы сборки внутри каталога `build/`.

Флаг `-B build` указывает каталог сборки (build tree). Полная форма команды:

```
cmake -B build -S .
```

где `-S .` — каталог с исходниками (где лежит `CMakeLists.txt`). Если `-S` опущен, берётся текущий каталог. Каталог `build` создаётся автоматически, если его нет.

Здесь же передаются параметры конфигурации через `-D`:

```
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake -B build -DCMAKE_INSTALL_PREFIX=/usr/local
```

Выбор генератора через `-G`:

```
cmake -B build -G Ninja
cmake -B build -G "Unix Makefiles"
```

**Этап 2: `cmake --build build`**

Это этап сборки. Команда вызывает реальную систему сборки (Make, Ninja, MSBuild) в каталоге `build`, компилируя и линкуя цели. Главное преимущество — единый кроссплатформенный синтаксис: не нужно помнить, вызывать ли `make`, `ninja` или `msbuild` напрямую.

Полезные опции:

```
cmake --build build --target myapp    # собрать конкретную цель
cmake --build build -j 8              # параллельная сборка в 8 потоков
cmake --build build --config Release  # выбор конфигурации (для multi-config генераторов)
```

**Почему два этапа**

Конфигурация — дорогая операция (проверка компиляторов, поиск зависимостей), и её не нужно повторять при каждой сборке. После первого `cmake -B build` достаточно вызывать только `cmake --build build`. CMake сам перезапустит конфигурацию, если вы изменили `CMakeLists.txt`.

**Важно про `--config`**

Различают два типа генераторов. Single-config (Make, Ninja) — тип сборки задаётся на этапе конфигурации через `-DCMAKE_BUILD_TYPE`. Multi-config (Visual Studio, Ninja Multi-Config, Xcode) — тип выбирается на этапе сборки через `--config Release`, а `CMAKE_BUILD_TYPE` для них игнорируется.

**Типичный цикл**

```
cmake -B build -DCMAKE_BUILD_TYPE=Release   # один раз
cmake --build build                          # после каждого изменения кода
```
