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
#include <iostream>
#include <string>
#include <format>
#include <vector>
#include <unordered_map>
#include <functional>

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>

#include "user.pb.h"
#include "order.pb.h"

namespace {

    void printSetFields(const google::protobuf::Message& msg, const int indent = 0);

    std::unordered_map<
        google::protobuf::FieldDescriptor::CppType,
        std::function<bool(
            const google::protobuf::Reflection*,
            const google::protobuf::Message&,
            const google::protobuf::FieldDescriptor*,
            const int)>> HANDLERS = {

        {
            google::protobuf::FieldDescriptor::CPPTYPE_INT32,
            [](
                const google::protobuf::Reflection* reflection,
                const google::protobuf::Message& msg,
                const google::protobuf::FieldDescriptor* field,
                const int indent) {

                std::cout << reflection->GetInt32(msg, field);
                return false;
            }
        },
        {
            google::protobuf::FieldDescriptor::CPPTYPE_INT64,
            [](
                const google::protobuf::Reflection* reflection,
                const google::protobuf::Message& msg,
                const google::protobuf::FieldDescriptor* field,
                const int indent) {

                std::cout << reflection->GetInt64(msg, field);
                return false;
            }
        },
        {
            google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE,
            [](
                const google::protobuf::Reflection* reflection,
                const google::protobuf::Message& msg,
                const google::protobuf::FieldDescriptor* field,
                const int indent) {

                std::cout << reflection->GetDouble(msg, field);
                return false;
            }
        },
        {
            google::protobuf::FieldDescriptor::CPPTYPE_BOOL,
            [](
                const google::protobuf::Reflection* reflection,
                const google::protobuf::Message& msg,
                const google::protobuf::FieldDescriptor* field,
                const int indent) {

                std::cout << reflection->GetBool(msg, field) ? "true" : "false";
                return false;
            }
        },
        {
            google::protobuf::FieldDescriptor::CPPTYPE_STRING,
            [](
                const google::protobuf::Reflection* reflection,
                const google::protobuf::Message& msg,
                const google::protobuf::FieldDescriptor* field,
                const int indent) {

                std::cout << std::format("\"{}\"", reflection->GetString(msg, field));
                return false;
            }
        },
        {
            google::protobuf::FieldDescriptor::CPPTYPE_ENUM,
            [](
                const google::protobuf::Reflection* reflection,
                const google::protobuf::Message& msg,
                const google::protobuf::FieldDescriptor* field,
                const int indent) {

                std::cout << reflection->GetEnum(msg, field)->name();
                return false;
            }
        },
        {
            google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE,
            [](
                const google::protobuf::Reflection* reflection,
                const google::protobuf::Message& msg,
                const google::protobuf::FieldDescriptor* field,
                const int indent) {

                std::cout << '\n';
                printSetFields(reflection->GetMessage(msg, field), indent + 2);
                return true;
            }
        }
    };

    const auto DEFAULT_HANDLER = [](const google::protobuf::Reflection*,
                                    const google::protobuf::Message&,
                                    const google::protobuf::FieldDescriptor*,
                                    const int) {
        std::cout << "<NOT HANDLED IN DEMO>\n";
        return false;
    };

    // Полностью generic-функция: не знает ни про User, ни про Order,
    // ни про какой-либо другой конкретный тип сообщения. Работает через
    // базовый класс google::protobuf::Message + Reflection + Descriptor.
    void printSetFields(const google::protobuf::Message& msg, const int indent) {
        const google::protobuf::Descriptor* descriptor{msg.GetDescriptor()};
        const google::protobuf::Reflection* reflection{msg.GetReflection()};
        std::string pad(indent * 2, ' ');

        std::cout << std::format("{}{} {{\n", pad, descriptor->name());

        // ListFields() возвращает только УСТАНОВЛЕННЫЕ поля (в отличие от
        // перебора всех field_count() дескриптора, где пришлось бы руками
        // звать HasField для каждого)
        std::vector<const google::protobuf::FieldDescriptor*> fields;
        reflection->ListFields(msg, &fields);

        for (const auto* field: fields) {
            std::cout << std::format("{} {} (#{}, {}) =", pad, field->name(), field->number(), field->type_name());

            if (field->is_repeated()) {
                const int count{reflection->FieldSize(msg, field)};
                std::cout << std::format("[{} items]\n", count);
                if (field->cpp_type() ==
                    google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE) {
                    // рекурсивно печатаем первые пару вложенных сообщений
                    for (int i{}; i < std::min(count, 2); ++i) {
                        printSetFields(reflection->GetRepeatedMessage(msg, field, i), indent + 2);
                    }
                }
                continue;
            }

            auto handler = HANDLERS.contains(field->cpp_type()) ? HANDLERS[field->cpp_type()] : DEFAULT_HANDLER;
            if (handler(reflection, msg, field, indent)) continue;
            std::cout << '\n';
        }
        std::cout << std::format("{}\n", pad);
    }

    // Ещё один generic-инструмент: подсчёт установленных полей рекурсивно,
    // работает для ЛЮБОГО типа сообщения без единой строчки типоспецифичного кода.
    int countSetFieldsRecursive(const google::protobuf::Message& msg) {
        const google::protobuf::Reflection* reflection{msg.GetReflection()};
        std::vector<const google::protobuf::FieldDescriptor*> fields;
        reflection->ListFields(msg, &fields);

        int total{static_cast<int>(fields.size())};
        for (const auto* field: fields) {
            if (field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE) {
                if (field->is_repeated()) {
                    const int count{reflection->FieldSize(msg, field)};
                    for (int i{}; i < count; ++i) {
                        total += countSetFieldsRecursive(reflection->GetRepeatedMessage(msg, field, i));
                    }
                } else {
                    total += countSetFieldsRecursive(reflection->GetMessage(msg, field));
                }
            }
        }

        return total;
    }

}

int main() {
    myapp::User user;
    user.set_id(1);
    user.set_name("Alice");
    user.set_phone("+123");
    user.add_addresses()->set_city("Berlin");
    user.add_addresses()->set_city("Munich");

    myapp::Order order;
    order.set_order_id(42);
    order.set_total_amount(199.5);
    order.set_status(myapp::Order::ORDER_STATUS_PAID);
    order.add_items()->set_sku("A1");
    order.add_items()->set_sku("A2");

    std::cout << "=== PrintSetFields(user) - type defined in runtime ===\n";
    printSetFields(user);

    std::cout << "\n=== PrintSetFields(order) - same func other type ===\n";
    printSetFields(order);

    std::cout << "\n=== CountSetFieldsRecursive ===\n";
    std::cout << "user:  " << countSetFieldsRecursive(user) << " set fields (with nested)\n";
    std::cout << "order: " << countSetFieldsRecursive(order) << " set fields (with nested)\n";

    return 0;
}

```

**Суть Reflection API**

`GetReflection()` возвращает объект `Reflection` (единый на весь тип), `GetDescriptor()` — `Descriptor`, описывающий структуру сообщения (список полей, их типы, номера) — тот самый метаданные, что были зашиты в `.proto` файл, но теперь доступны в рантайме, а не только на этапе кодогенерации. Ключевая идея: код, написанный один раз против абстрактного `google::protobuf::Message`, работает с **любым** сгенерированным типом сообщения без перекомпиляции и без `if/switch` по конкретным классам.

В выводе видно: `PrintSetFields` вызвана один раз для `User`, один раз для `Order` — совершенно разные схемы (разные поля, разные номера, разная вложенность), но функция не содержит ни строчки, специфичной для `User` или `Order`.

**Как устроен обход**

`ListFields()` — самый практичный метод: возвращает только **установленные** поля (не нужно самому гонять `HasField()` по всем `field_count()` дескриптора). Для каждого `FieldDescriptor` дальше нужен `cpp_type()` (`CPPTYPE_INT32`, `CPPTYPE_STRING`, `CPPTYPE_MESSAGE`, `CPPTYPE_ENUM` и т.д.) — по нему выбирается нужный `Get*` метод (`GetInt32`, `GetString`, `GetMessage`, `GetEnum`...). Для `repeated`-полей — отдельная ветка через `FieldSize()` + `GetRepeatedMessage(msg, field, i)`/`GetRepeated*`.

Вложенные сообщения (`Address` внутри `User.addresses`, `Item` внутри `Order.items`) обходятся рекурсивно — `PrintSetFields` вызывает сама себя на `reflection->GetMessage(msg, field)`, поэтому вложенность любой глубины не требует ручной поддержки для каждого уровня.

**`CountSetFieldsRecursive` — вторая generic-утилита**

Ещё компактнее показывает идею: одна функция без единого упоминания `User`/`Order`/`Address`/`Item` посчитала 6 установленных полей у обоих сообщений (включая вложенные `Address`/`Item` внутри repeated-полей) — потому что вся логика опирается только на `Descriptor`/`Reflection`, а не на конкретные геттеры сгенерированного класса.

**Где это применяется на практике**

Именно на Reflection API построены: `DebugString()`/`Utf8DebugString()` (которые мы разбирали раньше), конвертация в JSON (`google::protobuf::util::MessageToJsonString`), text format парсер/принтер, gRPC reflection service (позволяет `grpcurl` узнавать структуру сервиса без .proto файлов на клиенте), универсальные библиотеки сравнения/диффа сообщений (`MessageDifferencer`), и generic-прокси/шины сообщений, которые пересылают protobuf-данные, не зная их конкретной схемы.

**Компромисс**

Reflection заметно медленнее прямых сгенерированных геттеров/сеттеров (виртуальные вызовы, диспетчеризация по типу поля в рантайме вместо инлайнящегося кода) — использовать его в горячем пути (сериализация каждого запроса в проде) не стоит; область применения — generic-инструменты, отладка, интроспекция, а не основной путь данных.
