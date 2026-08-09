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

### address.proto
```protobuf
syntax = "proto3";  
  
package myapp;  
  
option optimize_for = SPEED;  
  
message Address {  
  string street = 1;  
  string city = 2;  
  string postal_code = 3;  
  string country = 4;  
  // не у всех адресов есть квартира/офис  
  optional string apartment = 5;  
}
```

### user.proto
```protobuf
syntax = "proto3";  
  
package myapp;  
  
import "address.proto";  
  
option optimize_for = SPEED;  
  
message User {  
  enum Status {  
    STATUS_UNKNOWN = 0;  
    STATUS_ACTIVE = 1;  
    STATUS_SUSPEND = 2;  
    STATUS_DELETED = 3;  
  }  
  
  int32 id = 1;  
  string name = 2;  
  string email = 3;  
  bool is_verified = 4;  
  double balance = 5;  
  bytes avatar = 6;  
  Status status = 7;  
  repeated Address addresses = 8;  
  map<string, string> preferences = 9;  
  
  // ровно один способ связи должен быть указан  
  oneof contact_method {  
    string phone = 10;  
    string telegram_handle = 11;  
  }
}
```

### order.proto
```protobuf
syntax = "proto3";  
  
package myapp;  
  
import "user.proto";  
import "address.proto";  
  
option optimize_for = SPEED;  
  
message Order {  
  message Item {  
    string sku = 1;  
    string title = 2;  
    int32 quantity = 3;  
    double unit_price = 4;  
  }  
  
  enum OrderStatus {  
    ORDER_STATUS_UNKNOWN = 0;  
    ORDER_STATUS_PENDING = 1;  
    ORDER_STATUS_PAID = 2;  
    ORDER_STATUS_SHIPPED = 3;  
    ORDER_STATUS_CANCELLED = 4;  
  }  
  
  int64 order_id = 1;  
  User buyer = 2;  
  Address shipping_address = 3;  
  repeated Item items = 4;  
  OrderStatus status = 5;  
  double total_amount = 6;  
  map<string, string> metadata = 7;  
  // появляется только после отправки  
  optional string tracking_number = 8;  
}
```

### main.cpp
```cpp
#include <chrono>  
#include <iostream>  
#include <format>  
#include <memory>  
#include <vector>  
  
#include <google/protobuf/arena.h>  
  
#include "order.pb.h"  
  
constexpr int K_ORDERS{300'000}; // сколько Order создаём за прогон  
constexpr int K_ITEMS_PER_ORDER{8}; // вложенных Item на один Order  
  
namespace {  
    void fillOrder(myapp::Order* order, const int i, const int kItemsPerOrder) {  
        order->set_order_id(i);  
        order->set_total_amount(99.99);  
        order->set_status(myapp::Order::ORDER_STATUS_PAID);  
        for (int j{}; j < kItemsPerOrder; ++j) {  
            myapp::Order::Item* item{order->add_items()};  
            item->set_sku(std::format("SKU-{}", j));  
            item->set_title("Item title");  
            item->set_quantity(j + 1);  
            item->set_unit_price(9.99);  
        }    
    }  
    
    using Clock = std::chrono::steady_clock;  
    long ms_since(Clock::time_point start) {  
        return std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - start).count();  
    }  
    
    void logDuration(const std::string& label, long create_ms, const long destroy_ms) {  
        std::cout << std::format("{}: creation: {} ms, destroying: {} ms, total {} ms\n",  
            label,  
            create_ms,  
            destroy_ms,  
            create_ms + destroy_ms);  
    }  
    
    std::tuple<int, int> getParams(const int argc, char** argv) {  
        int bufferOrders{-1};  
        if (argc > 1) {  
            try {  
                bufferOrders = std::stoi(argv[1]);  
            } catch (...) {}  
        }  
        int bufferItemsPerOrder{-1};  
        if (argc > 2) {  
            try {  
                bufferItemsPerOrder = std::stoi(argv[2]);  
            } catch (...) {}  
        }  
        int kOrders{bufferOrders >= 0 ? bufferOrders : K_ORDERS};  
        int kItemsPerOrder{bufferItemsPerOrder >= 0 ? bufferItemsPerOrder : K_ITEMS_PER_ORDER};  
  
        return {kOrders, kItemsPerOrder};  
    }  
}  
  
int main(const int argc, char *argv[]) {  
    const auto [kOrders, kItemsPerOrder] = getParams(argc, argv);  
  
    std::cout << std::format("{} Order x {} Item = {} protobuf-objects\n",  
        kOrders,  
        kItemsPerOrder,  
        static_cast<long long>(kOrders) * (kItemsPerOrder + 1));  
  
    // ===== 1. Куча: раздельно меряем создание и уничтожение =====  
    {  
        std::vector<myapp::Order*> orders;  
        orders.reserve(kOrders);  
  
        const auto t_create = Clock::now();  
        for (int i{}; i < kOrders; ++i) {  
            myapp::Order* order{new myapp::Order()};  
            fillOrder(order, i, kItemsPerOrder);  
            orders.push_back(order);  
        }        
        const long create_ms{ms_since(t_create)};  
  
        const auto t_destroy{Clock::now()};  
        for (const auto* order : orders) delete order; // рекурсивный обход дерева + N+M free()  
        const long destroy_ms{ms_since(t_destroy)};  
  
        logDuration("Heap", create_ms, destroy_ms);  
    }
  
    // ===== 2. Arena: раздельно меряем создание и уничтожение =====  
    {  
        auto arena{std::make_unique<google::protobuf::Arena>()};  
        std::vector<myapp::Order*> orders;  
        orders.reserve(kOrders);  
  
        const auto t_create = Clock::now();  
        for (int i{}; i < kOrders; ++i) {  
            myapp::Order* order{  
                google::protobuf::Arena::Create<myapp::Order>(arena.get())  
            };            
            fillOrder(order, i, kItemsPerOrder);  
            orders.push_back(order);  
        }        
        const long create_ms{ms_since(t_create)};  
  
        const auto t_destroy{Clock::now()};  
        arena.reset(); // одно освобождение памяти всей арены целиком  
        const long destroy_ms{ms_since(t_destroy)};  
  
        logDuration("Arena", create_ms, destroy_ms);  
    }  
    return 0;  
}
```

**Зачем нужна Arena**

`google::protobuf::Arena` — это пул памяти: большой блок выделяется одним `malloc`, а все сообщения (в том числе вложенные, созданные через `add_items()`/`mutable_*()` на арена-аллоцированном родителе) "нарезаются" из этого блока bump-pointer'ом — просто сдвигом указателя, без похода в аллокатор на каждый объект. Освобождение — не рекурсивный обход дерева с вызовом `delete` на каждом узле, а один проход по блокам арены.

**Что реально показал бенчмарк**

При 300 000 `Order` × 8 `Item` (2.7 млн protobuf-объектов), стабильно на трёх прогонах:

```
Куча:   создание=~205 ms, уничтожение=~52 ms, итого=~255 ms
Arena:  создание=~97 ms,  уничтожение=~53 ms, итого=~150 ms
```

Создание через Arena — **вдвое быстрее** кучи. Это и есть основной эффект: `malloc` на каждый маленький объект делает работу по поиску подходящего чанка, аудит по free-листам, возможную синхронизацию (даже thread-local кэш glibc — tcache — имеет ограниченный размер на класс размера, и при миллионах аллокаций постоянно выходит за него); Arena вместо этого просто двигает указатель внутри уже выделенного блока — O(1) без похода в общий аллокатор в большинстве случаев.

**Неожиданность: уничтожение не стало быстрее**

Ожидалось, что Arena выиграет ещё сильнее на освобождении (один `Reset`/деструктор арены вместо 2.7 млн `delete`), но в этой версии protobuf (3.12.4) время уничтожения оказалось практически одинаковым. Причина — `Item`/`Order` содержат `string`-поля (`sku`, `title`): даже на арене protobuf вынужден регистрировать нетривиальные деструкторы для таких полей и пройтись по ним при уничтожении арены, так что "бесплатного" освобождения дерева тут не вышло. В более новых версиях protobuf (ArenaStringPtr был существенно доработан) этот разрыв обычно заметнее.

**Почему на 50 000 объектах Arena проигрывала**

При меньшем масштабе (мой первый, неудачный прогон) собственные накладные расходы Arena — выделение первого блока, бухгалтерия для отслеживания деструкторов строковых полей — перевешивали выигрыш от bump-pointer аллокации, а glibc-аллокатор на таком объёме прекрасно справлялся сам через tcache. Вывод: Arena — это оптимизация для **больших** графов сообщений (десятки/сотни тысяч и больше вложенных объектов за раз — типичная ситуация в высоконагруженном gRPC-сервере, где на каждый запрос строится большое дерево сообщений), а не универсальное ускорение "на любой случай". На маленьких графах разница может быть незаметна или даже отрицательна.

**Практическое правило**

Arena стоит использовать, когда: сообщения создаются и уничтожаются массово и быстро (request/response cycle сервера), граф сообщений большой и глубоко вложенный, и/или нужно избежать фрагментации кучи в долгоживущем процессе с высокой пропускной способностью. Для одиночных сообщений или редких операций Arena не даёт ощутимой выгоды и добавляет сложности (нельзя просто `delete`, нужно следить за временем жизни арены относительно всех указателей на её объекты).
