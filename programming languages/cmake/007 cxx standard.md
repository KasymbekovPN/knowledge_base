---
tags:
  - programming-language
  - cmake
---
[[programming languages/cmake/_|<=]]

Есть два способа задать требуемый стандарт C++: через переменную `CMAKE_CXX_STANDARD` (грубо, для всего проекта) и через `target_compile_features` (точечно, для конкретной цели и с передачей требования потребителям). Современный подход предпочитает второй.

## Способ 1: `CMAKE_CXX_STANDARD`

Набор переменных, управляющих стандартом C++ для всех целей, объявленных после их установки.

```cmake
set(CMAKE_CXX_STANDARD 17)            # требуемый стандарт
set(CMAKE_CXX_STANDARD_REQUIRED ON)   # это требование, а не пожелание
set(CMAKE_CXX_EXTENSIONS OFF)         # -std=c++17, а не -std=gnu++17
```

Разберём три переменные:

`CMAKE_CXX_STANDARD` — номер стандарта (`11`, `14`, `17`, `20`, `23`).

`CMAKE_CXX_STANDARD_REQUIRED` — если `ON`, CMake выдаст ошибку, когда компилятор не поддерживает стандарт. Если `OFF` (по умолчанию), CMake молча «откатится» на более старый стандарт — почти всегда нежелательное поведение, поэтому ставьте `ON`.

`CMAKE_CXX_EXTENSIONS` — если `OFF`, используется чистый стандарт (`-std=c++17`) без расширений компилятора (`-std=gnu++17`). Рекомендуется `OFF` для переносимости.

**Минус подхода:** это глобальные переменные. Они не передаются потребителям библиотеки и должны быть установлены **до** объявления целей. Это «грубый» инструмент, как и другие глобальные команды.

## Способ 2: `target_compile_features`

Привязывает требование к конкретной цели и, что важно, передаёт его потребителям через ключевые слова видимости. Вы указываете не номер стандарта напрямую, а нужную возможность языка — CMake сам подберёт подходящий стандарт.

```cmake
target_compile_features(mylib PUBLIC cxx_std_17)
```

`cxx_std_17` — мета-фича, означающая «нужен минимум C++17». Любая цель, слинкованная с `mylib`, автоматически получит требование C++17 — не нужно дублировать настройку.

Видимость работает как обычно: PUBLIC — стандарт нужен и библиотеке, и потребителям (например, если он «протекает» через публичные заголовки); PRIVATE — только при сборке самой цели; INTERFACE — только потребителям (для header-only).

Доступны мета-фичи `cxx_std_11`, `cxx_std_14`, `cxx_std_17`, `cxx_std_20`, `cxx_std_23`. Можно указывать и конкретные фичи (`cxx_constexpr`, `cxx_lambdas` и др.), но на практике почти всегда используют именно `cxx_std_NN`.

## Сравнение

| |`CMAKE_CXX_STANDARD`|`target_compile_features`|
|---|---|---|
|Область действия|Глобально (все цели ниже)|Конкретная цель|
|Передаётся потребителям|Нет|Да (PUBLIC/INTERFACE)|
|Контроль видимости|Нет|Да|
|Стиль|Старый|Современный (target-based)|

## Пример: target-based подход

```
mathlib/
├── CMakeLists.txt
├── include/mathlib/vec.hpp
├── src/vec.cpp
└── app/main.cpp
```

### include/mathlib/vec.hpp (использует C++17 в публичном API)
```cpp
#pragma once  
  
#include <optional>  
  
std::optional<double> safe_divide(double, double);
```

### src/vec.cpp
```cpp
#include "mathlib/vec.hpp"  
  
#include <optional>  
  
std::optional<double> safe_divide(const double a, const double b) {  
    if (b == 0.0) return std::nullopt;  
    return a / b;  
}
```

### app/main.cpp
```cpp
#include <mathlib/vec.hpp>  
  
#include <iostream>  
#include <optional>  
  
int main() {  
    if (auto result = safe_divide(10.0, 3.0)) {  
        std::cout << *result << '\n';  
    }
}
```

```
3.33333
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 4.0.0)  
project(MathDemo LANGUAGES CXX)  
  
add_library(mathlib STATIC src/vec.cpp)  
target_include_directories(mathlib PUBLIC include)  
  
# Публичный API использует std::optional → C++17 нужен и потребителям.  
# Поэтому PUBLIC — требование «протечёт» к app автоматически.  
target_compile_features(mathlib PUBLIC cxx_std_17)  
  
add_executable(app app/main.cpp)  
target_link_libraries(app PRIVATE mathlib)  
# app НЕ задаёт стандарт явно — наследует C++17 от mathlib
```

Цель `app` не содержит ни единой строчки про стандарт C++, но соберётся с C++17, потому что требование пришло от `mathlib` по графу зависимостей.

## Пример: глобальный подход (для сравнения)

```cmake
cmake_minimum_required(VERSION 3.16)
project(MathDemo LANGUAGES CXX)

# Задаём ДО объявления целей — действует на все
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(mathlib STATIC src/vec.cpp)
target_include_directories(mathlib PUBLIC include)

add_executable(app app/main.cpp)
target_link_libraries(app PRIVATE mathlib)
```

Работает, но требование C++17 нигде не «прикреплено» к `mathlib`. Если эту библиотеку использует другой проект, ему придётся самому помнить про стандарт — он не передаётся вместе с целью.

## Практическое правило

Для библиотек, которые будут использоваться другими, применяйте `target_compile_features` с PUBLIC (если стандарт виден в заголовках) или PRIVATE (если только в реализации) — так требование путешествует вместе с целью.

`CMAKE_CXX_STANDARD` удобен как быстрая установка дефолта во всём проекте — например, в небольшом приложении без переиспользуемых библиотек. Многие комбинируют оба: ставят глобальные переменные как «базовую планку» и уточняют требования отдельным целям через `target_compile_features`.
