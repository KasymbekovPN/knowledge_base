---
tags:
  - protobuf
---
[[programming languages/protobuf/_|<=]]

### vcpkg
```json
{  
  "name" : "protobuf-file-demo",  
  "version" : "1.0.0",  
  "dependencies" : ["protobuf"]  
}
```

### CMakePresets.json
```json
{  
    "version": 6,  
    "configurePresets": [  
        {            
	        "name": "base",  
            "hidden": true,  
            "generator": "Visual Studio 18 2026",  
            "architecture": {  
                "value": "x64",  
                "strategy": "set"  
            },  
            "binaryDir": "${sourceDir}/build/${presetName}",  
            "toolchainFile": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake",  
            "cacheVariables": {  
                "VCPKG_TARGET_TRIPLET": "x64-windows-static-md"  
            }  
        },        
        {            
	        "name": "debug",  
            "inherits": "base"  
        },  
        {            
	        "name": "release",  
            "inherits": "base"  
        }  
    ],    
    "buildPresets": [  
        {            
	        "name": "debug",  
            "configurePreset": "debug",  
            "configuration": "Debug"  
        },  
        {            
	        "name": "release",  
            "configurePreset": "release",  
            "configuration": "Release"  
        }  
    ]}
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 4.4.2)  
project(protobuf_file_demo CXX)  
  
find_package(Protobuf REQUIRED)  
  
set(PROTO_FILES address.proto user.proto order.proto)  
add_library(proto_gen OBJECT ${PROTO_FILES})  
target_link_libraries(proto_gen PUBLIC protobuf::libprotobuf)  
target_compile_features(proto_gen PUBLIC cxx_std_23)  
  
protobuf_generate(  
        TARGET proto_gen  
        LANGUAGE cpp  
        IMPORT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}  
        PROTOC_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}  
)  
target_include_directories(proto_gen PUBLIC ${CMAKE_CURRENT_BINARY_DIR})  
  
add_executable(file_demo main.cpp)  
target_link_libraries(file_demo PRIVATE proto_gen)  
target_include_directories(file_demo PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
```

### main.cpp
```cpp
#include <chrono>  
#include <iostream>  
#include <format>  
  
#include "order.pb.h"  
  
// элементов в одном большом Order  
constexpr int K_ITEMS{200'000};  
  
namespace {  
    myapp::Order buildBigOrder(const int order_quantity) {  
        myapp::Order order;  
        order.set_order_id(1);  
        for (int i{}; i < order_quantity; ++i) {  
            myapp::Order::Item* item = order.add_items();  
            item->set_sku(std::format("SKU-{}", i));  
            item->set_title("Item title");  
            item->set_quantity(i);  
            item->set_unit_price(9.99);  
        }  
        // никакой ручной работы — компилятор сам выберет move/RVO  
        return order;  
    }  
    
    using Clock = std::chrono::steady_clock;  
    long ns_since(const Clock::time_point start) {  
        return std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - start).count();  
    }  
    
    std::tuple<int> getParams(const int argc, char** argv) {  
        int bufferItems{-1};  
        if (argc > 1) {  
            try {  
                bufferItems = std::stoi(argv[1]);  
            } catch (...) {}  
        }  
        int kItems{bufferItems >= 0 ? bufferItems : K_ITEMS};  
  
        return {kItems};  
    }  
}  
  
int main(const int argc, char *argv[]) {  
    const auto [kItems] = getParams(argc, argv);  
  
    myapp::Order original{buildBigOrder(kItems)};  
    std::cout << std::format("Order with {} items is built\n", original.items_size());  
  
    // ===== 1. Копирование (CopyFrom / copy constructor) =====  
    {  
        const auto t{Clock::now()};  
        // вызывает copy constructor -> глубокая копия  
        myapp::Order copy{original};  
        const auto ns = ns_since(t);  
        std::cout << std::format("Copy ctor: {} ns, items in copy: {}\n", ns, copy.items_size());  
    }  
    // ===== 2. Move-конструктор =====  
    {  
        // сначала честно копируем, чтобы не портить original  
        myapp::Order source{original};  
        const auto t{Clock::now()};  
        // move constructor -> перенос владения буфером  
        myapp::Order moved{std::move(source)};  
        const auto ns{ns_since(t)};  
        std::cout << std::format("Move ctor: {} ns, items in moved: {}, items in source: {}\n",  
            ns,  
            moved.items_size(),  
            source.items_size());  
    }  
    // ===== 3. Swap() =====  
    {  
        myapp::Order a{original};  
        myapp::Order b;  
        b.set_order_id(999);  
  
        const auto t{Clock::now()};  
        // обмен внутренними указателями, без копирования содержимого  
        a.Swap(&b);  
        const auto ns{ns_since(t)};  
        std::cout << std::format("Swap: {} ns, items in b: {}, items in a: {}\n", ns, b.items_size(), a.items_size());  
    }  
    // ===== 4. Move-присваивание (operator=(Order&&)) =====  
    {  
        myapp::Order source{original};  
        myapp::Order target;  
        const auto t{Clock::now()};  
        // move assignment  
        target = std::move(source);  
        const auto ns{ns_since(t)};  
        std::cout << std::format("Move assignment {} ns, items in target: {}\n", ns, target.items_size());  
    }  
    // ===== 5. Возврат по значению из функции: RVO/move, без лишней копии =====  
    {  
        const auto t{Clock::now()};  
        // построение + возврат  
        myapp::Order returned{buildBigOrder(kItems)};  
        const auto ns{ns_since(t)};  
        std::cout << std::format("Builder {} ns, items in returned: {}\n", ns, returned.items_size());  
    }  
    return 0;  
}
```


**Copy constructor**

`myapp::Order copy(original)` вызывает сгенерированный copy constructor, который рекурсивно копирует всё дерево: 200 000 `Item`, каждый со своими `string`-полями — полноценная глубокая копия с аллокациями на каждый элемент.

**Move constructor**

```cpp
myapp::Order moved(std::move(source));
```

После move `source.items_size() == 0` — сообщение-источник не скопировано, а **опустошено**: внутренние указатели на буферы (repeated-поле с 200 000 элементами) просто переданы новому объекту, старый остался в валидном, но пустом состоянии. В сгенерированном коде move-конструктор и move-присваивание (`operator=(Order&&)`) появляются автоматически начиная с protobuf, поддерживающего C++11 move semantics — переносятся внутренние указатели `RepeatedPtrField`, `ArenaStringPtr` и т.д., без копирования содержимого.

**`Swap()`**

```cpp
a.Swap(&b);
```

Не move-конструкция, а явный обмен двух уже существующих объектов — тоже O(1) относительно размера данных, потому что меняются местами внутренние указатели/дескрипторы, а не байты. После свопа `b` содержит 200 000 items (раньше принадлежавших `a`), а `a` получила `order_id=999`, который раньше был у `b`. Исторически `Swap()` — это API, появившийся в protobuf ещё до того, как в язык C++ вошли move-семантика (C++11); сейчас он остаётся полезен там, где нужен именно обмен, а не перенос-и-опустошение (например, переиспользование уже выделенного объекта без новых аллокаций в hot path).

**Move assignment**

Тот же эффект, что move-конструктор, но для `target = std::move(source)` — сработал автоматически сгенерированный `operator=(Order&&)`.

**Возврат по значению — 12 ms на всю функцию (включая построение 200 000 items)**

```cpp
myapp::Order BuildBigOrder() {
  myapp::Order order;
  // ... заполнение 200 000 items ...
  return order;
}
```

Никакого ручного `std::move(order)` не нужно — компилятор применяет RVO (return value optimization) или, если оптимизация невозможна, автоматически выберет move-конструктор для локальной переменной при `return`. Раньше (до C++11 и до move-семантики в protobuf) типичный API выглядел бы как `void BuildBigOrder(Order* out)` именно чтобы избежать копирования при возврате — с move-семантикой в этом больше нет нужды, `Order` можно возвращать по значению без штрафа.

**Практическое правило**

Не нужно вручную "оптимизировать" передачу protobuf-сообщений через указатели/output-параметры из страха перед копированием — генерируемый код уже поддерживает дешёвые move/`Swap()`. Копирование (`CopyFrom`, copy constructor, `operator=(const T&)`) остаётся дорогим и должно использоваться осознанно — только когда действительно нужны два независимых экземпляра данных.
