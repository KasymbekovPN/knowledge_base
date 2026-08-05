---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/attributes/_|<=]]

`__attribute__((weak))` — помечает функцию или переменную как "слабый" символ: если в процессе линковки встречается другое, "сильное" (обычное) определение с тем же именем, линкер использует сильное и молча отбрасывает слабое, без ошибки multiple definition. Если сильного определения нигде нет — используется слабое (или, если объявлена только слабая декларация без определения, символ резолвится в `nullptr`/адрес 0, что можно проверить в рантайме).

```cpp
// libfoo — предоставляет реализацию по умолчанию
__attribute__((weak))
void onError(int code) {
    std::fprintf(stderr, "default handler: error %d\n", code);
}

void doWork() {
    if (/* что-то пошло не так */ true) {
        onError(42);
    }
}
```

```cpp
// приложение, использующее libfoo, может переопределить onError
// своей "сильной" версией — без изменения кода библиотеки:
void onError(int code) {
    std::cerr << "custom handler: " << code << "\n";
}
```

Проверка наличия слабого символа перед вызовом (для опциональных зависимостей):

```cpp
extern "C" __attribute__((weak)) void optional_feature();

void run() {
    if (optional_feature) {          // если символ не найден линкером — nullptr
        optional_feature();
    } else {
        std::cout << "optional_feature not linked, skipping\n";
    }
}
```

Типичные применения:

- **Переопределяемые дефолты в библиотеках** — обработчики ошибок, хуки, weak-версии функций malloc/free для инструментации памяти (valgrind, jemalloc, tcmalloc используют этот приём, чтобы приложение могло само подставить свою реализацию `malloc`, если она есть, иначе — стандартную).
- **Опциональные зависимости** — код компилируется независимо от того, слинкована ли конкретная библиотека, а в рантайме проверяет, доступна ли функция.
- **Embedded/прошивки** — weak-реализации обработчиков прерываний (`__attribute__((weak)) void IRQHandler() { /* default: do nothing */ }`), которые пользователь переопределяет в своём коде, только если нужно кастомное поведение — иначе линкуется дефолтная заглушка.

Осторожности:

- Стандартного `[[...]]`-аналога нет — чисто GCC/Clang; на MSVC для похожих задач используют `#pragma comment(linker, "/alternatename:...")` (гораздо менее удобно).
- Поведение специфично для конкретного формата объектных файлов (ELF на Linux, Mach-O на macOS немного иначе трактует weak-символы) — не всегда 100% переносимо даже между Unix-подобными системами.
- В C++ (в отличие от C) слабые символы чаще встречаются на уровне extern "C" функций — для C++ функций с манглингом и шаблонов есть смежная, но другая механика (`weak` linkage для inline-функций и шаблонных инстанциаций генерируется компилятором автоматически, без явного атрибута, чтобы избежать multiple-definition ошибок между единицами трансляции).

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.30)  
project(demo CXX)  
  
add_executable(demo main.cpp libfoo.cpp)  
target_include_directories(demo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)  
target_compile_features(demo PUBLIC cxx_std_23)
```

### include/libfoo.hpp
```cpp
#pragma once  
  
// Определения onError (weak) и process() — в libfoo.cpp, в отдельной  
// единице трансляции. Weak/strong резолвится линкером МЕЖДУ разными  
// единицами трансляции: если положить weak-определение прямо в  
// заголовок и подключить его туда же, где лежит "сильный" onError,  
// компилятор увидит два определения в одном .obj и выдаст обычную  
// ошибку redefinition, а не отбросит слабое.  
void onError(int code);  
void process(int value);
```

### libfoo.cpp
```cpp
#include "include/libfoo.hpp"  
  
#include <format>  
#include <iostream>  
  
__attribute__((weak))  
void onError(const int code) {  
    std::cout << std::format("[weak] code: {}\n", code);  
}  
  
void process(const int value) {  
    if (value <= 0) {  
        onError(-1);  
        return;  
    }  
    std::cout << "[ok]\n";  
}
```

### main.cpp
```cpp
#include "include/libfoo.hpp"  
  
#include <iostream>  
#include <format>  
  
void onError(const int code) {  
    std::cout << std::format("[strong] code: {}\n", code);  
}  
  
int main() {  
    process(100);  
    process(-100);  
  
    return 0;  
}
```
