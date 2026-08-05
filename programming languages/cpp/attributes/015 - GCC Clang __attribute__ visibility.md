---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/attributes/_|<=]]

`__attribute__((visibility("...")))` (`[[gnu::visibility("...")]]`) — контролирует, какие символы (функции, классы, глобальные переменные) экспортируются из shared-библиотеки (`.so`/`.dylib`) во внешний ABI, а какие остаются внутренними. Актуально только на ELF-платформах (Linux, macOS) — на Windows аналогичную роль играют `__declspec(dllexport)`/`__declspec(dllimport)`.

Значения:

- `"default"` — символ виден снаружи библиотеки (обычное поведение по умолчанию, если не менять флаги сборки).
- `"hidden"` — символ не экспортируется, виден только внутри самой библиотеки.
- `"protected"` — виден снаружи, но не может быть переопределён (interposed) другой библиотекой.
- `"internal"` — ещё строже `hidden`, обычно не используется вручную.

```cpp
// публичный API библиотеки
__attribute__((visibility("default")))
void publicFunction();

// внутренняя реализация, не должна попадать в экспортируемый ABI
__attribute__((visibility("hidden")))
void internalHelper();
```

Типичный паттерн — компилировать всё с `-fvisibility=hidden` (делает `hidden` поведением по умолчанию для всех символов), а публичный API помечать явно через макрос:

```cpp
#if defined(_WIN32)
  #define API_EXPORT __declspec(dllexport)
#else
  #define API_EXPORT __attribute__((visibility("default")))
#endif

API_EXPORT void doWork();   // единственная точка входа, видимая извне
```

Зачем это нужно:

- **Ускорение загрузки и линковки** — меньше символов в динамической таблице, быстрее resolve.
- **Меньший размер бинарника** и меньше шансов на конфликт имён с другими библиотеками.
- **Реальная инкапсуляция ABI** — внутренние детали реализации физически недоступны извне, а не просто "не задокументированы".
- **Оптимизации компилятора** — `hidden`-символы компилятор может инлайнить/оптимизировать агрессивнее, зная, что на них не может быть внешних ссылок через `dlsym` или interposition.

На практике почти всегда используется через обёрточный макрос (как в примере выше), потому что чистый `visibility` — GCC/Clang-специфика, а `dllexport`/`dllimport` на Windows устроены иначе (там ещё и разные атрибуты для экспорта и импорта одного и того же символа).

## Структура

```
.../
├── CMakeLists.txt          # top-level: подключает подпроекты, задаёт hidden-by-default
├── mylib/
│   ├── CMakeLists.txt      # add_library(mylib SHARED ...)
│   ├── include/mylib/mylib.h
│   └── src/mylib.cpp
└── app/
    ├── CMakeLists.txt      # add_executable(app ...), линкуется с mylib
    └── main.cpp
```

## Сборка

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/app/app
```

## Проверка экспортируемых символов (Linux/macOS)

```bash
nm -D --defined-only build/mylib/libmylib.so | c++filt
```

Ожидаемый результат: видны только `Calculator::Calculator`, `Calculator::add`, `Calculator::multiply`, `freeFunctionAdd` — то, что помечено `MYLIB_API`. `internalHelper` и `internalOnlyFunction` в списке быть не должно.

## Нюанс, который проверен и подтверждён сборкой

`private` в C++ — это проверка доступа компилятором, она не влияет на то, попадёт ли символ в динамическую таблицу .so. Метод класса, помеченного `MYLIB_API` (т.е. `visibility("default")`), наследует видимость класса и экспортируется, даже если объявлен `private` — если явно не переопределить видимость самого метода через `MYLIB_HIDDEN` (`__attribute__((visibility("hidden")))`).

В `mylib.h` `internalHelper` намеренно помечен `MYLIB_HIDDEN`, чтобы показать разницу — без этой пометки он появлялся в `nm -D` наравне с публичными методами, несмотря на `private`.

Без `CMAKE_CXX_VISIBILITY_PRESET hidden` (или без ручных visibility-атрибутов) GCC/Clang по умолчанию экспортируют из `.so` вообще все не-static символы — это раздувает динамическую таблицу символов и мешает компилятору агрессивно оптимизировать/инлайнить внутренние функции библиотеки.

### mylib/include/mylib/mylib.hpp
```cpp
#pragma once  
  
// Кроссплатформенный макрос экспорта символов.  
// Windows (MSVC/MinGW): __declspec(dllexport|dllimport)  
// Linux/macOS (GCC/Clang): __attribute__((visibility("default")))  
#if defined(_WIN32) || defined(__CIGWIN)  
    #if defined(MYLIB_EXPORTS)  
        #define MYLIB_API __declspec(dllexport)  
    #else  
        #define MYLIB_API __declspec(dllimport)  
    #endif  
    #define MYLIB_HIDDEN  
#else  
    #if defined(MYLIB_EXPORTS)  
        #define MYLIB_API __attribute__((visibility("default")))  
    #else        #define MYLIB_API    #endif    #define MYLIB_HIDDEN __attribute__((visibility("hidden")))  
#endif  
  
// Публичный класс библиотеки — часть ABI.  
class MYLIB_API Calculator {  
public:  
    Calculator();  
  
    [[nodiscard]] int add(int, int) const;  
    [[nodiscard]] int multiply(int, int) const;  
  
private:  
    // ВАЖНЫЙ НЮАНС: методы класса, помеченного MYLIB_API, по умолчанию  
    // наследуют visibility класса и ВСЁ РАВНО экспортируются, даже если    
    // объявлены private — "private" это доступ на уровне C++ (компилятор),    
    // а visibility — это видимость символа на уровне линкера/ABI, они    
    // никак не связаны. Чтобы реально скрыть метод из экспортируемых    
    // символов .so, нужно явно пометить его MYLIB_HIDDEN.    
    MYLIB_HIDDEN [[nodiscard]] int internalHelper(int) const;  
};  
  
// Публичная свободная функция — тоже часть ABI.  
MYLIB_API int freeFunctionAdd(int, int);  
  
// Явно внутренняя функция: не экспортируется из .so вообще,  
// даже если случайно попадёт в заголовок. Полезно для хелперов,  
// которые должны быть недоступны через dlsym()/линковку извне,  
// но по каким-то причинам не static и не в анонимном namespace.  
MYLIB_HIDDEN void internalOnlyFunction();
```

### mylib/src/mylib.cpp
```cpp
#include "mylib/mylib.hpp"  
  
#include <iostream>  
  
Calculator::Calculator() {  
    std::cout << "[mylib] Calculator created\n";  
}  
  
int Calculator::add(const int a, const int b) const {  
    return a + b;  
}  
int Calculator::multiply(const int a, const int b) const {  
    return internalHelper(a) * b;  
}  
  
int Calculator::internalHelper(const int x) const {  
    return x;  
}  
  
int freeFunctionAdd(const int a, const int b) {  
    return a + b;  
}  
  
void internalOnlyFunction() {  
    std::cout << "[mylib] internal function, not exported from the shared library\n";  
}
```

### mylib/CMakeLists.txt
```cmake
add_library(mylib SHARED  
        src/mylib.cpp  
)  
target_include_directories(mylib PUBLIC  
        ${CMAKE_CURRENT_SOURCE_DIR}/include  
)  
  
# Определён только при сборке самой библиотеки -> переключает  
# MYLIB_API на dllexport (Windows) / visibility("default") (Unix).  
# Потребители библиотеки (app) этот define не видят -> получают dllimport.  
target_compile_definitions(mylib PRIVATE MYLIB_EXPORTS)
```

### app/main.cpp
```cpp
#include <mylib/mylib.hpp>  
  
#include <iostream>  
#include <format>  
  
int main() {  
    const Calculator calc;  
  
    std::cout << std::format("add: {}\n", calc.add(2, 3));  
    std::cout << std::format("multiply: {}\n", calc.multiply(2, 3));  
    std::cout << std::format("freeFunctionAdd: {}\n", freeFunctionAdd(2, 3));  
  
    // internalOnlyFunction(); // error  
  
    return 0;  
}
```

### app/CMakeLists.txt
```cmake
add_executable(app main.cpp)  
  
target_link_libraries(app PRIVATE mylib)  
  
# На Windows загрузчик ищет DLL рядом с exe; без копирования  
# mylib.dll запуск падает с 0xc0000135 (DLL not found).  
if(WIN32)  
    add_custom_command(TARGET app POST_BUILD  
            COMMAND ${CMAKE_COMMAND} -E copy_if_different  
            $<TARGET_FILE:mylib>            $<TARGET_FILE_DIR:app>    )  
endif()
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.40)  
project(visibility_demo CXX)  
  
set(CMAKE_CXX_STANDARD 23)  
set(CMAKE_CXX_STANDARD_REQUIRED ON)  
  
# По умолчанию все символы в shared-библиотеках делаем скрытыми (hidden),  
# экспортируем только то, что явно помечено visibility("default") /  
# MYLIB_API. Это применяется ко всем целям проекта.  
set(CMAKE_CXX_VISIBILITY_PRESET hidden)  
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)  
  
add_subdirectory(mylib)  
add_subdirectory(app)
```
