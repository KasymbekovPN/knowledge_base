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
project(protobuf_incremental_demo CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Protobuf REQUIRED)

set(PROTO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/proto)
set(PROTO_FILES
        ${PROTO_ROOT}/common/address.proto
        ${PROTO_ROOT}/users/user.proto
        ${PROTO_ROOT}/orders/order.proto
)

# Генерируем в поддиректорию build-папки, структура повторяет IMPORT_DIRS:
# build/generated/common/address.pb.h, build/generated/users/user.pb.h, ...
set(PROTO_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)

add_library(proto_gen OBJECT ${PROTO_FILES})
target_link_libraries(proto_gen PUBLIC protobuf::libprotobuf)

protobuf_generate(
        TARGET proto_gen
        LANGUAGE cpp
        IMPORT_DIRS ${PROTO_ROOT}
        PROTOC_OUT_DIR ${PROTO_GEN_DIR}
        # флаг нужен только для protobuf < 3.15 (proto3 optional был experimental);
        # для свежего protobuf из vcpkg этот флаг не требуется
        # PROTOC_OPTIONS --experimental_allow_proto3_optional
)

target_include_directories(proto_gen PUBLIC ${PROTO_GEN_DIR})

add_executable(incr_demo main.cpp)
target_link_libraries(incr_demo PRIVATE proto_gen)

```

### main.cpp
```cpp
#include <iostream>
#include "orders/order.pb.h"

int main() {
    myapp::orders::Order order;
    order.set_order_id(1);
    order.mutable_buyer()->set_name("Alice");
    order.mutable_buyer()->mutable_address()->set_city("Berlin");
    std::cout << order.Utf8DebugString();

    return 0;
}

```

**Организация в отдельной директории**

```
proto/
  common/address.proto    (package myapp.common)
  users/user.proto        (package myapp.users, import "common/address.proto")
  orders/order.proto      (package myapp.orders, import "users/user.proto")
```

В `CMakeLists.txt` — `IMPORT_DIRS ${PROTO_ROOT}` (корень `proto/`), пути в `import` пишутся относительно этого корня (`"common/address.proto"`, а не полный путь). protoc транслирует `package myapp.common` в C++ namespace `myapp::common`, независимо от физической раскладки по папкам — совпадение структуры директорий со структурой пакетов (как у нас) чисто конвенция для читаемости, а не техническое требование.

**Генерация в build-директорию с сохранением структуры**

`PROTOC_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated` — protoc воссоздал ту же иерархию внутри build-папки: `generated/common/address.pb.h`, `generated/users/user.pb.h`, `generated/orders/order.pb.h` (видно в выводе `find`). Поэтому `#include "orders/order.pb.h"` в `main.cc` работает — путь относительно `IMPORT_DIRS`/`PROTOC_OUT_DIR`, а не физического расположения `main.cc`. Единственная деталь, которая доставила проблем именно со старым protoc 3.12.4: он не создаёт вложенные директории вывода сам (`No such file or directory`) — пришлось предварительно сделать `mkdir -p generated/{common,users,orders}`. В современных версиях protoc (используемых через vcpkg/Conan) это исправлено, директории создаются автоматически.

**Инкрементальная пересборка — два независимых уровня**

Уровень 1, кодогенерация (`protoc`): CMake добавляет `add_custom_command` с `DEPENDS` на конкретный `.proto`-файл — protoc перезапускается только для изменённого файла. В выводе видно: при правке `order.proto` строка `Running cpp protocol buffer compiler` появилась только один раз, именно для него.

Уровень 2, компиляция C++ (обычные правила заголовочных зависимостей make/ninja): если поменять `address.proto` (от которого зависят и `user.proto`, и транзитивно `order.proto`), протоc regenerates только `address.pb.h`/`.cc`, но C++ компилятор пересобирает **все** объектники, чьи `.cc`-файлы транзитивно инклюдят `address.pb.h` — в тесте выше пересобрались все три `.pb.cc.o`, потому что `user.pb.h` инклюдит `address.pb.h`, а `order.pb.h` инклюдит `user.pb.h`. Это стандартное поведение build-системы для C++, не специфика protobuf — просто цепочка `#include` в сгенерированном коде отражает цепочку `import` в схеме.

**Практический вывод**

Разбивка большой схемы на маленькие `.proto`-файлы с точечными импортами — не только про читаемость, но и про скорость итерации: правка "листового" файла (`order.proto`, от которого ничего не зависит) пересобирает минимум кода; правка "корневого", широко импортируемого файла (`address.proto`) неизбежно тянет пересборку всего, что от него зависит — это стоит учитывать при проектировании структуры схемы, стараясь не делать часто изменяемые поля частью широко переиспользуемых базовых сообщений.
