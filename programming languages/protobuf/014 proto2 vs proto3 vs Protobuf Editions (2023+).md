---
tags:
  - protobuf
---
[[programming languages/protobuf/_|<=]]

**proto2**

Синтаксис `syntax = "proto2";`. Presence всегда явная (explicit) для всех полей — `has_foo()` генерируется для скаляров, есть `default = "N/A"` для дефолтных значений. Есть `required` — поле, отсутствие которого при парсинге считается ошибкой; на практике признан антипаттерном (нельзя ни добавить, ни убрать `required`-поле без риска сломать совместимость) и в новых схемах не используется. `repeated`-поля скаляров **не** упакованы (packed) по умолчанию — нужно было явно писать `[packed=true]`. Enum — закрытый (closed): получение неизвестного значения enum считается ошибкой на уровне типа.

**proto3**

Синтаксис `syntax = "proto3";`. Presence по умолчанию **неявная** (implicit) — скаляры не имеют `has_foo()`, поле "не установлено" неотличимо от поля со значением по умолчанию (0, "", false). Явную presence возвращает ключевое слово `optional` (стабилизировано с 3.15, до этого — экспериментальный флаг, который мы использовали раньше на старом protoc). `repeated`-поля упакованы по умолчанию. `required` отсутствует вовсе. Enum — открытый (open): неизвестное значение просто хранится как число, не роняет парсинг.

**Protobuf Editions (с 2023, последняя — 2024)**

Заменяют деление на proto2/proto3 единым синтаксисом: `edition = "2024";` вместо `syntax = "...";`. Идея: вместо двух зашитых диалектов — набор независимых **features** (`field_presence`, `repeated_field_encoding`, `enum_type`, `utf8_validation` и т.д.), каждый со своим дефолтным значением для данной edition; любой feature можно переопределить точечно на уровне файла/сообщения/поля.

Ключевое для presence: `features.field_presence` принимает `EXPLICIT`, `IMPLICIT`, `LEGACY_REQUIRED`. В Edition 2023/2024 дефолт для полей — `EXPLICIT` (поведение как в proto2, без нужды писать `optional` — оно уже включено по умолчанию для всех полей). Чтобы получить proto3-подобное поведение для конкретного поля, пишут `[features.field_presence = IMPLICIT]` — как в нашем `id = 2 [features.field_presence = IMPLICIT]`. `required` как ключевое слово убран; та же функциональность доступна через `LEGACY_REQUIRED` — явно, как осознанный выбор, а не синтаксический дефолт.

Другие грамматические изменения 2024: `reserved gender;` без кавычек (было `reserved "gender";`), `import weak` убран в пользу `import option`, `ctype` убран в пользу `features.(pb.cpp).string_type`, добавлены `export`/`local` для контроля видимости символов между файлами.

**Практический вывод**

Wire-формат, text-формат и JSON-сериализация не меняются между proto2/proto3/Editions — старые и новые файлы можно свободно импортировать друг в друга (мы это тоже подтвердили: старый protoc спокойно понимает proto2/proto3-файлы, просто ничего не знает про `edition =`). Для новых проектов на актуальном protoc (v26+/2024 или новее) официальная рекомендация Google — использовать Editions вместо proto3; для нашего C++ учебного трека это пока не критично, поскольку системный protoc из apt (3.12.4) Editions не поддерживает — понадобится либо собрать protobuf из исходников свежей версии, либо ставить через vcpkg/Conan, где обычно уже актуальная версия.

Файлы: `player_proto2.proto`, `player_proto3.proto`, `player_editions.proto`.День 3 плана закрыт. Дальше — практика дня 3 (намеренно сломать совместимость и посмотреть на поведение), либо переходим к дню 4 (arena allocation, производительность). Что дальше?

Sources:

- [Protobuf Editions are here: don't panic · Buf](https://buf.build/blog/protobuf-editions-are-here)
- [Protobuf Editions Overview | Protocol Buffers Documentation](https://protobuf.dev/editions/overview/)
- [Feature Settings for Editions | Protocol Buffers Documentation](https://protobuf.dev/editions/features/)

### vcpkg.json
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
    ]
}
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 4.4.2)  
project(protobuf_file_demo CXX)  
  
find_package(Protobuf REQUIRED)  
  
set(PROTO_FILES player_proto2.proto player_proto3.proto player_editions.proto)  
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

### player_proto2.proto
```protobuf
// proto2: presence всегда явная (explicit), required реально существует  
syntax = "proto2";  
package com.example.v2;  
  
message Player {  
  optional string name = 1 [default = "N/A"];  
  required int32 id = 2; // required — только в proto2, признан антипаттерном  
  repeated  int32 scores = 3; // НЕ packed по умолчанию в proto2  
  
  enum Handed {  
    HANDED_UNSPECIFIED = 0;  
    HANDED_LEFT = 1;  
    HANDED_RIGHT = 2;  
  }  optional Handed handed = 4; // enum в proto2 закрытый (closed)  
  
  reserved "gender";  
}
```

### player_proto3.proto
```protobuf
// proto3: presence по умолчанию неявная (implicit) для скаляров,  
// явная presence только если явно написать `optional`  
syntax = "proto3";  
package com.example.v3;  
  
message Player {  
  optional string name = 1; // explicit presence, есть has_name()  
  int32 id = 2; // implicit presence, has_id() НЕТ  
  repeated int32 scores = 3; // packed по умолчанию в proto3  
  
  enum Handed {  
    HANDED_UNSPECIFIED = 0;  
    HANDED_LEFT = 1;  
    HANDED_RIGHT = 2;  
  }  optional Handed handed = 4; // enum в proto3 открытый (open)  
  
  reserved "gender";  
}
```

### player_editions.proto
```protobuf
// Editions: единый синтаксис вместо proto2/proto3. Presence по умолчанию  
// EXPLICIT для всех полей (как в proto2) — переопределяется поточечно  
// через features.field_presence, а не через выбор всего "диалекта" файла.  
edition = "2024";  
package com.example.ed;  
  
message Player {  
  string name = 1 [default = "N/A"]; // EXPLICIT по умолчанию, has_name()  
  int32 id = 2 [features.field_presence = IMPLICIT]; // явно просим implicit, как в proto3  
  repeated int32 scores = 3; // PACKED по умолчанию в 2024  
  
  enum Handed {  
    HANDED_UNSPECIFIED = 0;  
    HANDED_LEFT = 1;  
    HANDED_RIGHT = 2;  
  }  Handed handed = 4;  
  
  reserved  gender; // без кавычек — новая грамматика editions  
}
```

### main.cpp
```cpp
#include <iostream>  
#include <format>  
#include "player_proto2.pb.h"  
#include "player_proto3.pb.h"  
#include "player_editions.pb.h"  
  
int main() {  
  
    auto p2 = com::example::v2::Player();  
    p2.set_id(42);  
    std::cout << std::format("v2:\n{}\n", p2.Utf8DebugString());  
  
    auto p3 = com::example::v3::Player();  
    p3.set_id(43);  
    std::cout << std::format("v3:\n{}\n", p3.Utf8DebugString());  
  
    auto pe = com::example::ed::Player();  
    pe.set_id(44);  
    std::cout << std::format("ed:\n{}\n", pe.Utf8DebugString());  
  
    return 0;  
}
```
