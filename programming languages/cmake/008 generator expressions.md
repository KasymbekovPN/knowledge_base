---
tags:
  - programming-language
  - cmake
---
[[programming languages/cmake/_|<=]]

Генераторные выражения (generator expressions) — это конструкции вида `$<...>`, которые вычисляются **не** во время конфигурации, а позже — на этапе генерации файлов сборки. Это позволяет принимать решения, зависящие от вещей, которые на момент чтения `CMakeLists.txt` ещё неизвестны: конфигурации (Debug/Release), компилятора, платформы, свойств цели.

## Почему они нужны: две фазы CMake

Ключ к пониманию — CMake работает в две фазы:

**Фаза конфигурации** — читается `CMakeLists.txt`, выполняются команды, раскрываются обычные переменные `${...}`. Здесь ещё неизвестна конфигурация сборки в multi-config генераторах (Visual Studio, Xcode), потому что одна генерация создаёт сразу Debug и Release.

**Фаза генерации** — создаются конкретные файлы сборки, и вот тут раскрываются `$<...>`. Только на этом этапе известно, для какой именно конфигурации собирается цель.

Поэтому такой код **не работает** так, как ожидается в multi-config генераторах:

```cmake
# ❌ CMAKE_BUILD_TYPE пуст в multi-config генераторах на этапе конфигурации
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(app PRIVATE DEBUG_MODE)
endif()
```

А генераторное выражение — работает, потому что вычисляется позже, когда конфигурация уже определена:

```cmake
# ✅ решение принимается на этапе генерации, отдельно для каждой конфигурации
target_compile_definitions(app PRIVATE $<$<CONFIG:Debug>:DEBUG_MODE>)
```

## Базовый синтаксис

Общая форма — `$<ВЫРАЖЕНИЕ>`. Основные категории:

**Условные (булевы) выражения** возвращают `1` или `0`:

```cmake
$<CONFIG:Debug>                    # 1, если конфигурация Debug
$<CXX_COMPILER_ID:GNU>             # 1, если компилятор GCC
$<PLATFORM_ID:Windows>             # 1, если целевая ОС Windows
$<BOOL:${SOME_VAR}>                # приводит значение к 0/1
```

**Условная подстановка** — `$<условие:значение>` подставляет `значение`, только если условие истинно, иначе пустую строку:

```cmake
$<$<CONFIG:Debug>:-g>              # флаг -g только в Debug
$<$<CONFIG:Release>:-O3>           # флаг -O3 только в Release
```

**Тернарный выбор** — `$<IF:условие,да,нет>`:

```cmake
$<IF:$<CONFIG:Debug>,debug_lib,release_lib>
```

**Информационные выражения** возвращают значение:

```cmake
$<TARGET_FILE:mylib>               # полный путь к бинарнику цели
$<TARGET_PROPERTY:mylib,NAME>      # свойство цели
$<CXX_COMPILER_ID>                 # имя компилятора строкой
```

## Частые сценарии использования

**Флаги по конфигурации:**

```cmake
target_compile_options(app PRIVATE
    $<$<CONFIG:Debug>:-g;-O0>
    $<$<CONFIG:Release>:-O3;-DNDEBUG>
)
```

**Флаги по компилятору** (самый распространённый случай — разные ключи у GCC/Clang и MSVC):

```cmake
target_compile_options(app PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)
```

**Разные пути для сборки и установки** (обязательны при экспорте библиотек):

```cmake
target_include_directories(mylib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
```

`$<BUILD_INTERFACE:...>` действует, пока библиотека собирается внутри проекта; `$<INSTALL_INTERFACE:...>` — когда её используют после `install`. Так один и тот же `target_include_directories` корректно работает в обоих контекстах.

**Комбинирование условий:**

```cmake
$<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:GNU>>:-flto>   # LTO только в Release на GCC
$<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:-g>       # -g в двух конфигурациях
$<$<NOT:$<CONFIG:Debug>>:-DNDEBUG>                          # везде, кроме Debug
```

## Логические операторы

```cmake
$<AND:c1,c2,...>       # 1, если все истинны
$<OR:c1,c2,...>        # 1, если хотя бы один истинен
$<NOT:c>               # инвертирует
$<STREQUAL:a,b>        # 1, если строки равны
```

## Важные тонкости

Точка с запятой внутри `$<...:a;b;c>` — это разделитель списка. `$<$<CONFIG:Debug>:-g;-O0>` подставит два отдельных флага `-g` и `-O0`.

Генераторные выражения **нельзя** использовать в `message()`, `if()` и других командах, выполняемых на этапе конфигурации — там они не раскрываются и попадут в вывод как есть. Они работают только в свойствах целей: `target_compile_options`, `target_link_libraries`, `target_include_directories`, `install(...)` и подобных.

Вложенность допустима и часто необходима: `$<$<AND:...>:...>` — внешнее выражение подстановки содержит внутреннее булево.

## Когда применять

Используйте генераторные выражения, когда решение зависит от конфигурации, компилятора, платформы или свойств цели — особенно в multi-config генераторах, где обычный `if(CMAKE_BUILD_TYPE ...)` не сработает. Для простых вещей, известных на этапе конфигурации (например, значение `option`), достаточно обычного `if` — не усложняйте без необходимости.

## Практический пример

### main.cpp
```cpp
#include <iostream>  
  
int main() {  
    std::cout << "Hello World!\n";  
  
#ifdef ENABLE_ASSERTS  
    std::cout << "ENABLE_ASSERTS.\n";  
#endif  
  
#ifdef NDEBUG  
    std::cout << "NDEBUG.\n";  
#endif  
  
    return 0;  
}
```

```
Hello World!
ENABLE_ASSERTS.
```

### CMakeLists.txt
```cpp
cmake_minimum_required(VERSION 4.0.0)  
project(GenExtDemo LANGUAGES CXX)  
  
add_executable(app main.cpp)  
  
target_compile_features(app PUBLIC cxx_std_20)  
  
# 1. Оптимизация и макросы по конфигурации  
target_compile_options(app PRIVATE  
        $<$<CONFIG:Debug>:-O0;-g>  
        $<$<CONFIG:Release>:-O3>)  
target_compile_definitions(app PRIVATE  
        $<$<CONFIG:Debug>:ENABLE_ASSERTS>  
        $<$<CONFIG:Release>:NDEBUG>        $<$<PLATFORM_ID:Windows>:WIN32_LEAN_AND_MEAN> # Флаг только для конкретной платформы  
)  
  
# 2. Предупреждения в зависимости от компилятора  
target_compile_options(app PRIVATE  
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Waa;-Wextra;-Wpedantic>  
        $<$<CXX_COMPILER_ID:MSVC>:/W4>)
```

Один `CMakeLists.txt` корректно адаптируется под любую комбинацию конфигурации, компилятора и платформы, без единого `if`.
