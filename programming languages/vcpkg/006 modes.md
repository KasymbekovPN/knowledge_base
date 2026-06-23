---
tags:
  - build
---
[[programming languages/vcpkg/_|<=]]

# Два режима работы: classic vs manifest

vcpkg может работать двумя способами. Понимание разницы важно, потому что от режима зависит, где живут зависимости и как они подключаются.

## Classic mode (классический режим)

Исторически первый режим. Пакеты устанавливаются **глобально** в каталог самого vcpkg (`installed/`), и любой проект на машине может ими пользоваться.

```bash
vcpkg install fmt zlib curl
```

Как это работает:

- Вы вручную вызываете `vcpkg install` для нужных библиотек.
- Они складываются в `<vcpkg>/installed/<триплет>/`.
- Проект подключается к этому общему хранилищу (через toolchain или `integrate install`).

Минусы:

- Зависимости **общие** для всех проектов. Проект A и проект B вынуждены делить одни и те же версии.
- Нет описания зависимостей вместе с проектом — кто-то должен помнить и вручную поставить нужный набор.
- Воспроизводимость хуже: «у меня собирается, у тебя нет», потому что у вас разный набор глобально установленного.

Когда уместен: быстрые эксперименты, разовые пробы библиотеки, обучение.

## Manifest mode (режим манифеста) — рекомендуемый

Зависимости описываются в файле **`vcpkg.json`**, который лежит рядом с проектом. vcpkg читает этот файл и ставит всё перечисленное **локально для проекта** — в каталог `vcpkg_installed/` рядом с ним, не трогая глобальное хранилище.

```
my_project/
├── vcpkg.json          ← список зависимостей
├── CMakeLists.txt
├── CMakePresets.json
└── vcpkg_installed/    ← зависимости именно этого проекта (создаётся автоматически)
```

Как это работает:

- Вы описываете зависимости в `vcpkg.json`.
- При конфигурации CMake (с подключённым toolchain) vcpkg **автоматически** ставит их в `vcpkg_installed/`.
- Команды `install`/`remove` вручную звать не нужно — источник истины это манифест.

Плюсы:

- **Изоляция**: у каждого проекта свой набор и свои версии. Проект A и B не мешают друг другу.
- **Воспроизводимость**: манифест коммитится в git, и любой, кто склонировал проект, получает ровно те же зависимости.
- **Версионирование**: можно зафиксировать конкретные версии через baseline и `version>=` (отдельный пункт плана).
- Декларативность: видно зависимости проекта, просто открыв `vcpkg.json`.

Когда уместен: практически всегда для реальных проектов. Это современный рекомендуемый подход.

## Сравнение

| |Classic mode|Manifest mode|
|---|---|---|
|Где зависимости|Глобально в `<vcpkg>/installed/`|Локально в `vcpkg_installed/` проекта|
|Описание|Нет файла, вручную `install`|Файл `vcpkg.json`|
|Изоляция проектов|Нет, общие|Да, у каждого свои|
|Воспроизводимость|Слабая|Сильная (манифест в git)|
|Версионирование|Ограничено|Полноценное (baseline, overrides)|
|Установка|Вручную `vcpkg install`|Автоматически при CMake-конфигурации|
|Когда|Эксперименты|Реальные проекты (рекомендуется)|

## Минимальный `vcpkg.json`

```json
{
  "dependencies": [ "fmt" ]
}
```

Расширенный вариант с метаданными и фиксацией baseline:

```json
{
  "name": "my-project",
  "version": "0.1.0",
  "dependencies": [
    "fmt",
    { "name": "curl", "features": ["ssl"] }
  ],
  "builtin-baseline": "<git-хеш-коммита-vcpkg>"
}
```

`builtin-baseline` фиксирует, из какого состояния реестра vcpkg брать версии — это и даёт воспроизводимость. Подробнее в пункте про версионирование.

## Что запомнить

Classic mode — глобальные пакеты, ручная установка, для экспериментов. Manifest mode — зависимости в `vcpkg.json` рядом с проектом, ставятся автоматически при сборке, изолированы и воспроизводимы. Для любого серьёзного проекта используйте manifest mode.

# Пример

Структура:

```
shapes-demo/
├── vcpkg.json
├── CMakeLists.txt
├── CMakePresets.json
├── include/
│   └── shapes.h
└── src/
    ├── shapes.cpp
    └── main.cpp
```

## `vcpkg.json` — манифест зависимостей

```json
{
  "name": "shapes-demo",
  "version": "0.1.0",
  "dependencies": [
    "fmt"
  ]
}
```

Один пункт — библиотека `fmt`. vcpkg установит её автоматически при конфигурации CMake.

## `include/shapes.h`

```cpp
#pragma once

namespace shapes {

struct Point {
    double x{};
    double y{};
};

class Rect {

public:
    Rect(const Point& _top_left, const Point& _bottom_right):
        top_left_{_top_left},
        bottom_right_{_bottom_right} {}

    double width() const;
    double height() const;
    double area() const;
    double perimeter() const;
    Point center() const;

    const Point& top_left() const { return top_left_; }
    const Point& bottom_right() const { return bottom_right_; }

private:
    Point top_left_;
    Point bottom_right_;
};

} // namespace shapes
```

## `src/shapes.cpp`

```cpp
#include "shapes.h"

#include <cmath>

namespace shapes {

double Rect::width() const {
    return std::abs(bottom_right_.x - top_left_.x);
}

double Rect::height() const {
    return std::abs(top_left_.y - bottom_right_.y);
}

double Rect::area() const {
    return width() * height();
}

double Rect::perimeter() const {
    return 2.0 * (width() + height());
}

Point Rect::center() const {
    return Point{
        (top_left_.x + bottom_right_.x) / 2.0,
        (top_left_.y + bottom_right_.y) / 2.0
    };
}

} // namespace shapes
```

## `src/main.cpp`

Здесь подключаем `fmt` и используем его форматирование. Заодно покажу аккуратный приём — кастомный форматтер `fmt` для нашего `Point`, чтобы его можно было печатать напрямую.

```cpp
/*

# из каталога shapes-demo

cmake --preset default     # на этом шаге vcpkg сам поставит fmt в vcpkg_installed/

cmake --build build        # компиляция проекта

  

# запуск

./build/shapes-demo        # Linux/macOS

.\build\shapes-demo.exe    # Windows

*/

#include "shapes.h"

#include <fmt/core.h>
#include <fmt/format.h>

template <>
struct fmt::formatter<shapes::Point> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    auto format(const shapes::Point& p, format_context& ctx) const {
        return fmt::format_to(ctx.out(), "({:.1f}, {:.1f})", p.x, p.y);
    }
};

int main() {
    using shapes::Point;
    using shapes::Rect;

    Point tl{0.0, 4.0};
    Point br{3.0, 0.0};
    Rect rect{tl, br};

    fmt::print("Rectangle corners: {} -> {}\n",
               rect.top_left(), rect.bottom_right());
    fmt::print("Width:     {:.2f}\n", rect.width());
    fmt::print("Height:    {:.2f}\n", rect.height());
    fmt::print("Area:      {:.2f}\n", rect.area());
    fmt::print("Perimeter: {:.2f}\n", rect.perimeter());
    fmt::print("Center:    {}\n", rect.center());

    return 0;
}
```

## `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.20)
project(shapes-demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# fmt, установленный vcpkg, находится через find_package
find_package(fmt CONFIG REQUIRED)

add_executable(shapes-demo
    src/main.cpp
    src/shapes.cpp
)

target_include_directories(shapes-demo PRIVATE include)

# Линкуем с fmt
target_link_libraries(shapes-demo PRIVATE fmt::fmt)
```

Два ключевых момента интеграции с vcpkg здесь:

- `find_package(fmt CONFIG REQUIRED)` — находит установленный vcpkg-пакет (флаг `CONFIG` важен, vcpkg поставляет CMake-конфиги пакетов).
- `target_link_libraries(... fmt::fmt)` — подключает библиотеку через её импортированную цель.

## `CMakePresets.json` — связываем CMake с vcpkg

Это место, где подключается toolchain-файл vcpkg. Именно он включает manifest-режим (автоустановку из `vcpkg.json`).

```json
{
  "version": 3,
  "configurePresets": [
    {
      "name": "default",
      "generator": "Ninja",
      "binaryDir": "${sourceDir}/build",
      "toolchainFile": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
    }
  ]
}
```

Здесь используется переменная окружения `VCPKG_ROOT`, которую вы задали при установке. Можно прописать абсолютный путь, но через `$env{VCPKG_ROOT}` пресет остаётся переносимым.

## Сборка и запуск

```bash
# из каталога shapes-demo
cmake --preset default     # на этом шаге vcpkg сам поставит fmt в vcpkg_installed/
cmake --build build        # компиляция проекта

# запуск
./build/shapes-demo        # Linux/macOS
.\build\shapes-demo.exe    # Windows
```

При первом `cmake --preset default` vcpkg прочитает `vcpkg.json`, увидит `fmt`, скачает и соберёт его в `vcpkg_installed/`, и только потом CMake сконфигурирует проект.

## Ожидаемый вывод

```
Rectangle corners: (0.0, 4.0) -> (3.0, 0.0)
Width:     3.00
Height:    4.00
Area:      12.00
Perimeter: 14.00
Center:    (1.5, 2.0)
```

## Что демонстрирует пример

- **Manifest mode на практике**: зависимость `fmt` объявлена в `vcpkg.json`, поставлена автоматически, никаких ручных `vcpkg install`.
- **Интеграция через toolchain**: `CMakePresets.json` подключает `vcpkg.cmake`, и `find_package` просто находит библиотеку.
- **Разделение на translation units**: `shapes.h`/`shapes.cpp` отдельно, `main.cpp` отдельно — типичная структура небольшого проекта.
- **Реальное использование библиотеки**: `fmt::print` и кастомный форматтер для своего типа.
