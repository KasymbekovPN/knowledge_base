---
tags:
  - programming-language
  - cmake
---
[[programming languages/cmake/_|<=]]

`find_package` — команда для поиска и подключения внешних зависимостей (уже установленных в системе библиотек: Boost, OpenSSL, Threads, Qt и т.д.). Она находит библиотеку, проверяет версию и делает её доступной для линковки. У неё есть два принципиально разных режима работы — **Module** и **Config**.

## Базовый синтаксис

```cmake
find_package(OpenSSL 3.0 REQUIRED)
```

Разберём аргументы:

`OpenSSL` — имя пакета (регистрозависимо).

`3.0` — минимально требуемая версия (необязательно).

`REQUIRED` — если пакет не найден, CMake остановится с ошибкой. Без этого слова ненайденный пакет — не ошибка, и нужно самому проверять результат через `if(OpenSSL_FOUND)`.

После успешного поиска обычно линкуются с предоставляемой пакетом целью:

```cmake
find_package(OpenSSL 3.0 REQUIRED)
target_link_libraries(app PRIVATE OpenSSL::SSL OpenSSL::Crypto)
```

## Два режима поиска

Ключевое, что нужно понять — как именно CMake находит информацию о пакете. Есть два механизма.

### Module mode (модульный режим)

CMake ищет файл с именем `Find<Имя>.cmake` — так называемый **find-модуль**. Это скрипт, который знает, как найти данную библиотеку: где искать заголовки, где библиотечные файлы, как определить версию.

Такие модули поставляются либо **вместе с CMake** (их десятки — `FindThreads.cmake`, `FindOpenSSL.cmake`, `FindZLIB.cmake`, `FindBoost.cmake` и др.), либо пишутся вручную и кладутся в проект.

Идея модульного режима: библиотека **сама ничего не знает про CMake** (например, старая C-библиотека, собранная обычным Makefile). За неё «переводчиком» выступает find-модуль, написанный кем-то со стороны.

Чтобы CMake нашёл ваши собственные find-модули, укажите путь к ним:

```cmake
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(MyCustomLib REQUIRED)   # ищет cmake/FindMyCustomLib.cmake
```

### Config mode (конфигурационный режим)

CMake ищет файл с именем `<Имя>Config.cmake` или `<имя>-config.cmake` — так называемый **package config file**. Этот файл **поставляется самой библиотекой** при её установке.

Идея: современная библиотека, собранная через CMake, при установке (`install`) сама генерирует конфигурационный файл, где точно описаны её цели, пути, зависимости и версия. Никакой сторонний «переводчик» не нужен — библиотека сама себя описывает для CMake.

Такие файлы обычно устанавливаются в стандартные места вроде `<prefix>/lib/cmake/<Имя>/`, и CMake ищет их там автоматически.

### Как CMake выбирает режим

По умолчанию CMake сначала пробует **Module mode** (ищет `Find<Имя>.cmake`), и если модуль не найден — переходит к **Config mode** (ищет `<Имя>Config.cmake`). Можно принудительно задать режим:

```cmake
find_package(Foo REQUIRED MODULE)   # только модульный режим
find_package(Foo REQUIRED CONFIG)   # только конфигурационный режим
```

## Сравнение режимов

|                               | Module mode                  | Config mode                    |
| ----------------------------- | ---------------------------- | ------------------------------ |
| Ищет файл                     | `Find<Имя>.cmake`            | `<Имя>Config.cmake`            |
| Кто написал файл              | CMake или вы                 | Сама библиотека при установке  |
| Знает ли библиотека про CMake | Не обязательно               | Да, собрана через CMake        |
| Точность информации           | Зависит от качества модуля   | Высокая (от автора библиотеки) |
| Типичные примеры              | Threads, ZLIB, старые C-либы | Qt6, fmt, spdlog, gRPC         |

Современная тенденция — библиотеки всё чаще поставляют собственные config-файлы, поэтому config mode становится предпочтительным. Module mode остаётся нужен для библиотек, которые сами не поддерживают CMake.

## Что даёт пакет после нахождения

Хорошо оформленный пакет предоставляет **импортированные цели** (imported targets) — их достаточно слинковать, и все нужные пути к заголовкам, флаги и зависимости подтянутся автоматически (тот же target-based принцип):

```cmake
find_package(fmt REQUIRED)
target_link_libraries(app PRIVATE fmt::fmt)   # заголовки и флаги придут сами
```

Кроме целей, пакеты часто заполняют переменные (это более старый стиль):

```cmake
find_package(OpenSSL REQUIRED)
# доступны: OpenSSL_FOUND, OPENSSL_VERSION,
#           OPENSSL_INCLUDE_DIR, OPENSSL_LIBRARIES
```

Предпочитайте импортированные цели переменным, когда пакет их предоставляет — это надёжнее и переносимее.

# Практические примеры

## Пример 1: системная библиотека (Threads, module mode)

### main.cpp
```cpp
#include <iostream>  
#include <format>  
#include <thread>  
#include <vector>  
  
void worker(int _id) {  
    std::cout << std::format("Thread {} is working\n", _id);  
}  
  
int main() {  
    std::vector<std::thread> threads;  
    for (int i{}; i < 5; ++i) threads.emplace_back(worker, i);  
    for (auto& t : threads) t.join();  
  
    std::cout << "Done\n";  
}
```

Библиотека `Threads::Threads` подтягивает `-pthread` на Linux, без которого `std::thread` не слинкуется.

```
Thread 0 is working
Thread 2 is working
Thread 1 is working
Thread 4 is working
Thread 3 is working
Done
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 4.0.0)  
project(ThreadDemo LANGUAGES CXX)  
  
# использует FindThreads.cmake из CMake  
find_package(Threads REQUIRED)  
  
add_executable(app main.cpp)  
target_link_libraries(app PRIVATE Threads::Threads)  
target_compile_features(app PUBLIC cxx_std_20)
```

## Пример 2: современная библиотека (fmt, config mode)

### main.cpp
```cpp
/*  
cmake -B .build -DCMAKE_TOOLCHAIN_FILE=C:\projects\vcpkg\scripts\buildsystems\vcpkg.cmake  
cmake --build .build  
*/  
  
#include <fmt/core.h>  
#include <fmt/color.h>  
  
int main() {  
    fmt::print("Hello, {}!\n", "world");  
  
    int x{42};  
    fmt::print("dec: {}, hex: {:#x}\n", x, x);  
  
    fmt::print(fg(fmt::color::green), "color text\n");  
}
```

Заголовки `fmt/core.h` находятся автоматически благодаря импортированной цели `fmt::fmt` — путь к ним прописан в config-файле библиотеки.

```
Hello, world!
dec: 42, hex: 0x2a
color text
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 4.0.0)  
project(FmtDemo LANGUAGES CXX)  
  
# ищет fmtConfig.cmake  
find_package(fmt 9.0 REQUIRED)  
  
add_executable(app main.cpp)  
target_link_libraries(app PRIVATE fmt::fmt)  
target_compile_features(app PUBLIC cxx_std_20)
```

## Пример 3: ZLIB (необязательная зависимость)

### main.cpp
```cpp
/*  
cmake -B .build -DCMAKE_TOOLCHAIN_FILE=C:\projects\vcpkg\scripts\buildsystems\vcpkg.cmake  
cmake --build .build  
*/  
  
#include <iostream>  
#include <format>  
  
#ifdef HAVE_ZLIB  
#include <zlib.h>  
#endif  
  
int main() {  
#ifdef HAVE_ZLIB  
    std::string data{"Example data for compression: repeating data data data"};  
  
    uLong src_len = data.size();
    uLong dst_len = compressBound(src_len);
    std::string compressed(dst_len, '\0');  
    if (compress(reinterpret_cast<Bytef*>(&compressed[0]), &dst_len,  
        reinterpret_cast<Bytef*>(data.data()), src_len) == Z_OK) {  
        std::cout << std::format("Compressed: {} -> {} byte", src_len, dst_len);
    } else {  
        std::cout << "Compression error";
    }
#else  
    std::cout << "ZLIB is unreachable";  
#endif  
}
```

Здесь макрос `HAVE_ZLIB`, заданный в CMake через `target_compile_definitions` при `ZLIB_FOUND`, управляет условной компиляцией. Если библиотека не найдена, программа собирается и работает без функциональности сжатия — код зависимости изолирован через `#ifdef`.

```
Compressed: 54 -> 50 byte
```

```cmake
cmake_minimum_required(VERSION 4.0.0)  
project(ZlibDemo LANGUAGES CXX)  
  
# без REQUIRED  
find_package(ZLIB)  
  
add_executable(app main.cpp)  
target_compile_features(app PUBLIC cxx_std_20)  
  
if(ZLIB_FOUND)  
    target_link_libraries(app PRIVATE ZLIB::ZLIB)  
    target_compile_definitions(app PRIVATE HAVE_ZLIB)  
    message(STATUS "ZLIB found!!!")  
else()  
    message(STATUS "ZLIB not found!!!")  
endif()
```

## Диагностика

Если пакет не находится, полезные приёмы:

Включить подробный вывод поиска:

```
cmake -B build --debug-find
```

Подсказать CMake, где искать (если библиотека в нестандартном месте):

```
cmake -B build -DCMAKE_PREFIX_PATH=/opt/mylib
```

`CMAKE_PREFIX_PATH` — главный рычаг для config mode: CMake ищет `<prefix>/lib/cmake/<Имя>/` внутри перечисленных путей.

## Практические правила

Всегда используйте импортированные цели (`Foo::Foo`), если пакет их предоставляет — это переносит требования вместе с зависимостью.

Ставьте `REQUIRED` для обязательных зависимостей — лучше явная ошибка на этапе конфигурации, чем непонятная ошибка линковки.

Указывайте минимальную версию, если код зависит от конкретного API.

Для установки самих библиотек в систему удобно использовать пакетный менеджер (vcpkg, Conan) — он раскладывает config-файлы так, что `find_package` находит их без ручной настройки путей.

**Ограничение `find_package`:** он находит только то, что **уже установлено** в системе. Если библиотеки нет, он её не скачает. Для автоматической загрузки и сборки зависимостей прямо в процессе конфигурации служит `FetchContent` — логичная следующая тема.
