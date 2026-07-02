---
tags:
  - programming-language
  - cmake
---
[[programming languages/cmake/_|<=]]

Конфигурация сборки определяет уровень оптимизации, наличие отладочной информации и набор макросов. CMake управляет этим по-разному в зависимости от типа генератора, и это главный источник путаницы.

## Стандартные конфигурации

CMake из коробки знает четыре типа:

`Debug` — без оптимизации (`-O0`), с отладочной информацией (`-g`). Для разработки и отладки.

`Release` — максимальная оптимизация (`-O3`), без отладочной информации, определён макрос `NDEBUG` (отключает `assert`). Для production.

`RelWithDebInfo` — оптимизация (`-O2`) плюс отладочная информация (`-g`). Компромисс: быстрый код, но можно отлаживать/профилировать. `NDEBUG` определён.

`MinSizeRel` — оптимизация по размеру (`-Os`). Для встраиваемых систем и ситуаций, где важен размер бинарника.

## Два типа генераторов — принципиальное различие

Способ задания конфигурации зависит от генератора, и это критически важно понимать.

**Single-config генераторы** (Unix Makefiles, Ninja) — одна генерация = одна конфигурация. Тип задаётся на этапе **конфигурации** через `CMAKE_BUILD_TYPE`:

```
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Чтобы получить другую конфигурацию, нужен отдельный каталог сборки:

```
cmake -B build-debug   -DCMAKE_BUILD_TYPE=Debug
cmake -B build-release -DCMAKE_BUILD_TYPE=Release
```

**Multi-config генераторы** (Visual Studio, Xcode, Ninja Multi-Config) — одна генерация содержит **все** конфигурации сразу. `CMAKE_BUILD_TYPE` для них **игнорируется**. Тип выбирается на этапе **сборки** через `--config`:

```
cmake -B build -G "Ninja Multi-Config"
cmake --build build --config Debug
cmake --build build --config Release      # тот же build/, другая конфигурация
```

Именно из-за этого различия проверка `if(CMAKE_BUILD_TYPE STREQUAL "Debug")` ненадёжна: в multi-config генераторах переменная пуста, и условие никогда не сработает. Вместо неё используйте генераторное выражение `$<$<CONFIG:Debug>:...>`, которое корректно работает в обоих случаях.

## Важная деталь: `CMAKE_BUILD_TYPE` по умолчанию пуст

Если для single-config генератора не задать `CMAKE_BUILD_TYPE`, он будет **пустым**. Это не «Debug» по умолчанию — это отсутствие каких-либо флагов оптимизации и отладки. Результат: код собирается без `-O` и без `-g`, что почти всегда нежелательно.

Поэтому распространённая практика — задавать значение по умолчанию, если пользователь ничего не указал:

```cmake
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Тип сборки" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY
        STRINGS Debug Release RelWithDebInfo MinSizeRel)
endif()
```

Условие `NOT CMAKE_CONFIGURATION_TYPES` защищает от применения этого в multi-config генераторах, где переменная и должна быть пустой. `set_property(... STRINGS ...)` добавляет выпадающий список вариантов в `cmake-gui`.

## Настройка флагов под конфигурацию

Для флагов, зависящих от конфигурации, есть два подхода.

**Переменные `CMAKE_<LANG>_FLAGS_<CONFIG>`** — CMake автоматически применяет их для соответствующей конфигурации. Значения по умолчанию уже заданы (`-O3 -DNDEBUG` для Release и т.д.), но их можно дополнить:

```cmake
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -DMY_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native")
```

**Генераторные выражения** (предпочтительно, target-based) — работают в обоих типах генераторов:

```cmake
target_compile_options(app PRIVATE
    $<$<CONFIG:Debug>:-O0;-g>
    $<$<CONFIG:Release>:-O3>
)
target_compile_definitions(app PRIVATE
    $<$<CONFIG:Debug>:ENABLE_LOGGING>
)
```

## Практический пример

Флаг `--verbose` покажет полные команды компилятора, включая `-O3 -DNDEBUG`.

### main.cpp
```cpp
/*  
  
cmake -B .build-dbg  
cmake --build .build-dbg --config Debug --verbose  
  
cmake -B .build-rel  
cmake --build .build-rel --config Release --verbose  
*/  
  
#include <iostream>  
  
int main() {  
    std::cout << "Hello World!\n";  
#ifdef ENABLE_ASSERTS  
    std::cout << "ENABLE_ASSERTS\n";  
#endif  
#ifdef VERBOSE_LOG  
    std::cout << "VERBOSE_LOG\n";  
#endif  
#ifdef NDEBUG  
    std::cout << "NDEBUG\n";  
#endif  
  
    return 0;  
}
```

### Debug
```
Hello World!
ENABLE_ASSERTS
VERBOSE_LOG
```

### Release
```cpp
Hello World!
NDEBUG
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 4.0.0)  
  
# Значение по умолчанию до project() — чтобы project() увидел его при настройке флагов  
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)  
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)  
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY  
            STRINGS Debug Release RelWithDebInfo MinSizeRel  
    )  
endif()  
  
project(ConfigDemo LANGUAGES CXX)  
  
add_executable(app main.cpp)  
target_compile_features(app PUBLIC cxx_std_20)  
  
# Настройки, зависящие от конфигурации — через генераторные выражения  
target_compile_definitions(app PRIVATE  
        $<$<CONFIG:Debug>:ENABLE_ASSERTS;VERBOSE_LOG>  
        $<$<CONFIG:Release>:NDEBUG>)  
  
# Выведем текущую конфигурацию (полезно для диагностики)  
message(STATUS "Тип сборки: ${CMAKE_BUILD_TYPE}")
```

## Практические правила

Всегда задавайте значение по умолчанию для `CMAKE_BUILD_TYPE` в single-config проектах, чтобы случайно не собрать неоптимизированный и неотлаживаемый бинарник.

Для настроек, зависящих от конфигурации, предпочитайте генераторные выражения `$<$<CONFIG:...>:...>` вместо `if(CMAKE_BUILD_TYPE ...)` — они работают и в single-, и в multi-config генераторах.

Держите отдельные каталоги сборки для разных конфигураций в single-config генераторах (`build-debug`, `build-release`) — это чище, чем переконфигурировать один каталог туда-обратно.

Для повседневной работы удобно фиксировать конфигурации в `CMakePresets.json` (следующая большая тема) — тогда не придётся каждый раз вручную вписывать `-DCMAKE_BUILD_TYPE`.

**Следующий практический шаг:** попробуйте собрать один и тот же проект в Debug и Release в двух разных каталогах, затем сравните размер получившихся бинарников и время работы — разница в оптимизации станет наглядной. После этого логично перейти к `find_package` и подключению внешних зависимостей.
