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
#include <format>

#include <google/protobuf/any.pb.h>
#include <google/protobuf/timestamp.pb.h>
#include <google/protobuf/duration.pb.h>
#include <google/protobuf/struct.pb.h>
#include <google/protobuf/empty.pb.h>
#include <google/protobuf/util/time_util.h>

#include "user.pb.h"

int main() {
    // ===== Any: контейнер для сообщения ЛЮБОГО типа + строка типа =====
    {
        myapp::User user;
        user.set_id(7);
        user.set_name("Alice");

        google::protobuf::Any any;
        // сериализует User внутрь Any + запоминает type_url
        any.PackFrom(user);

        std::cout << "=== Any ===\n";
        std::cout << std::format("type_url: {}\n", any.type_url());
        std::cout << std::format("Is<myapp::User>(): {}\n", any.Is<myapp::User>());

        myapp::User unpacked;
        bool ok{any.UnpackTo(&unpacked)};
        std::cout << std::format("UnpackTo ok: {}, name: {}\n\n", ok, unpacked.name());
    }

    // ===== Timestamp: точка во времени (секунды + наносекунды от эпохи) =====
    {
        google::protobuf::Timestamp ts{google::protobuf::util::TimeUtil::GetCurrentTime()};
        std::cout << "=== Timestamp ===\n";
        std::cout << std::format("seconds: {}, nanos: {}\n", ts.seconds(), ts.nanos());
        std::cout << std::format("like ISO8601: {}\n", google::protobuf::util::TimeUtil::ToString(ts));

        // из time_t в Timestamp и обратно
        google::protobuf::Timestamp fixed{google::protobuf::util::TimeUtil::SecondsToTimestamp(1700000000)};
        std::cout << std::format("fixed timestamp: {}\n\n", google::protobuf::util::TimeUtil::ToString(fixed));
    }

    // ===== Duration: промежуток времени (может быть отрицательным) =====
    {
        google::protobuf::Duration d{google::protobuf::util::TimeUtil::SecondsToDuration(90)};
        std::cout << "=== Duration ===\n";
        std::cout << std::format("90 seconds -> {}, like a string {}\n", d.seconds(), google::protobuf::util::TimeUtil::ToString(d));
        google::protobuf::Duration neg{google::protobuf::util::TimeUtil::SecondsToDuration(-30)};
        std::cout << std::format("-30 seconds -> {}\n\n", neg.seconds());
    }

    // ===== Struct: динамическая, нетипизированная структура (как JSON-объект) =====
    {
        google::protobuf::Struct s;
        auto& fields = *s.mutable_fields();

        fields["name"].set_string_value("Alice");
        fields["age"].set_number_value(30);
        fields["is_admin"].set_bool_value(false);

        google::protobuf::Value* tags = &fields["tags"];
        tags->mutable_list_value()->add_values()->set_string_value("vip");
        tags->mutable_list_value()->add_values()->set_string_value("beta");

        std::cout << "=== Struct (dynamic schema, without .proto) ===\n";
        std::cout << s.DebugString();
    }

    // ===== Empty: маркер "нет данных", часто как ответ RPC =====
    {
        google::protobuf::Empty e;
        std::string bytes;
        e.SerializeToString(&bytes);
        std::cout << "=== Empty ===\n";
        std::cout << "serialized size = " << bytes.size() << " bytes (always 0)\n";
    }

    return 0;
}

```

---


**`Any` — контейнер для сообщения произвольного типа**

```proto
message Any {
  string type_url = 1;  // "type.googleapis.com/myapp.User"
  bytes value = 2;       // сериализованные байты User
}
```

`PackFrom(user)` сериализует `User` внутрь `value` и записывает `type_url` (по умолчанию — `type.googleapis.com/<полное_имя_типа>`). Из вывода: `type_url = type.googleapis.com/myapp.User`. Дальше на другой стороне (которая может вообще не знать заранее, что там за тип) — `any.Is<myapp::User>()` проверяет соответствие типа по `type_url`, а `UnpackTo(&unpacked)` десериализует, только если тип совпал (вернул `ok=1`, `name=Alice`). Практическое применение: поле в сообщении, тип которого заранее неизвестен (плагины, generic event bus, произвольные payload'ы в шине сообщений) — `Any` даёт типобезопасную распаковку без ручного switch по строкам.

**`Timestamp` — точка во времени**

```proto
message Timestamp {
  int64 seconds = 1;  // с 1970-01-01T00:00:00Z
  int32 nanos = 2;
}
```

`TimeUtil::GetCurrentTime()` дал реальное текущее время (`2026-08-08T14:46:05Z`, сходится с датой сессии). `TimeUtil::SecondsToTimestamp(1700000000)` перевёл unix-время в читаемый ISO8601 (`2023-11-14T22:13:20Z`) — типобезопасная альтернатива хранению времени как `int64 unix_time = 1` без единиц измерения в самом типе.

**`Duration` — промежуток времени, может быть отрицательным**

```proto
message Duration {
  int64 seconds = 1;
  int32 nanos = 2;
}
```

`90s`/`-30s` в выводе — в отличие от `Timestamp`, это не точка на шкале, а разница между двумя точками; знак значим (истёкшее время, дедлайн в прошлом и т.п.).

**`Struct`/`Value`/`ListValue` — динамическая, "нетипизированная" структура**

Аналог JSON-объекта внутри protobuf-схемы: `map<string, Value>`, где `Value` — `oneof` из `null_value`/`number_value`/`string_value`/`bool_value`/`struct_value`/`list_value`. В выводе видно вложенный `list_value` с двумя `string_value`. Используется, когда схема заранее неизвестна (конфиги, метаданные, произвольные атрибуты) — компромисс: теряется строгая типизация ради гибкости, по сути "JSON внутри protobuf".

**`Empty` — маркер "нет данных"**

```proto
message Empty {}
```

Сериализованный размер — 0 байт (подтверждено в выводе), как и должно быть у сообщения без полей. Чаще всего встречается как тип ответа gRPC-метода, которому нечего возвращать (`rpc DeleteUser(UserId) returns (google.protobuf.Empty);`) — явный сигнал "успех, но данных нет", в отличие от `void` в обычном коде, который в protobuf-мире не существует как тип.

**Практический вывод**

Все пять типов уже скомпилированы прямо внутри `libprotobuf` — не нужно ничего генерировать самому, только `#include <google/protobuf/any.pb.h>` (и аналоги) и линковка на `libprotobuf`, которая уже есть в проекте. Используются как стандартные "кирпичики" в собственных `.proto`-схемах: `google.protobuf.Timestamp created_at = 5;` вместо изобретения своего формата времени.

--- 


## День 1: Основы protobuf и синтаксис .proto

- [x] Установка: `protobuf-compiler` (protoc) и `libprotobuf-dev` через пакетный менеджер, либо сборка из исходников / vcpkg / Conan. (2026.08.06)
- [x] Синтаксис proto3: `message`, скалярные типы (int32, int64, string, bytes, bool, float/double), номера полей и их роль в wire-формате. (2026.08.06)
- [x] `repeated`, `optional`, `enum`, вложенные сообщения, `oneof`, `map<K,V>`. (2026.08.07)
- [x] Импорты между .proto файлами, `package`, опции `option cc_enable_arenas`, `option optimize_for`. (2026.08.07)
- [x] Практика: описать 3-4 связанных сообщения (например, `User`, `Address`, `Order`) с разными типами полей. (2026.08.07)

## День 2: Генерация C++ кода и API сообщений

- [x] Команда `protoc --cpp_out=. file.proto`, разбор сгенерированных `.pb.h` / `.pb.cc`. (2026.08.07)
- [x] Сгенерированный класс: геттеры/сеттеры, `set_`, `mutable_`, `add_` (для repeated), `has_` (для optional/oneof). (2026.08.07)
- [x] Сериализация: `SerializeToString`, `ParseFromString`, `SerializeToOstream`, `SerializeToArray`. (2026.08.07)
- [x] Отладка: `DebugString()`, `Utf8DebugString()`. (2026.08.07)
- [x] Практика: собрать простую CMake-программу, которая создаёт сообщение, сериализует в файл и читает обратно. (2026.08.07)

## День 3: Wire-формат и эволюция схемы

- [x] Как устроен бинарный формат: tag-length-value, varint-кодирование, zigzag для signed-типов. (2026.08.07)
- [x] Совместимость: почему нельзя переиспользовать номера полей, как безопасно добавлять/удалять/переименовывать поля. (2026.08.07)
- [x] `reserved` для полей и номеров, работа с неизвестными полями (unknown fields). (2026.08.07)
- [x] Разница proto2 vs proto3 vs Protobuf Editions (2023+): `optional` в proto3, дефолтные значения, presence-семантика. (2026.08.07)

## День 4: Производительность и память в C++

- [x] Arena allocation: `google::protobuf::Arena`, зачем нужен, как ускоряет аллокации для больших графов сообщений. (2026.08.08)
- [x] Move-семантика в сгенерированном коде, `Swap()`, избегание лишних копий. (2026.08.08)
- [x] Reflection API (`google::protobuf::Message::GetReflection()`) — для generic-кода, работающего с произвольными типами сообщений. (2026.08.08)
- [x] `Any`, `Timestamp`, `Duration`, `Struct`, `Empty` из `google/protobuf/*.proto` (well-known types). (2026.08.08)

## День 5: Интеграция с CMake / сборочной системой

- [ ] `find_package(Protobuf REQUIRED)`, `protobuf_generate_cpp()`.
- [ ] Альтернатива: `FetchContent`/vcpkg для protobuf как зависимости.
- [ ] Организация .proto файлов в отдельной директории, генерация в build-директорию, инкрементальная пересборка.
- [ ] Практика: настроить чистый CMake-проект, который тянет protobuf через vcpkg/Conan и генерирует код автоматически при сборке.

## День 6-7: gRPC — основы

- [ ] Установка `grpc` и `grpc_cpp_plugin`.
- [ ] Синтаксис сервисов в .proto: `service`, `rpc`, четыре типа вызовов (unary, server streaming, client streaming, bidi streaming).
- [ ] Генерация: `--grpc_out` и `--plugin=protoc-gen-grpc-cpp`.
- [ ] Синхронный сервер/клиент на C++: `grpc::Server`, `ServerBuilder`, `ClientContext`, `Stub`.
- [ ] Практика: реализовать unary RPC (например, `GetUser(UserRequest) -> UserResponse`), поднять сервер и клиент локально.

## День 8: gRPC — streaming и асинхронность

- [ ] Server streaming и client streaming на практике (например, стрим логов).
- [ ] Bidi streaming.
- [ ] Асинхронный API (`CompletionQueue`) — на уровне понимания, без глубокого погружения.
- [ ] Обработка ошибок: `grpc::Status`, коды ошибок, deadlines/timeouts, retry-политики.
- [ ] Практика: добавить server-streaming метод в сервис дня 6-7.

## День 9: Продвинутые темы

- [ ] Interceptors в gRPC (аутентификация, логирование).
- [ ] TLS/mTLS для защищённых соединений.
- [ ] Reflection service и `grpcurl` для отладки без клиента.
- [ ] Версионирование API и backward-compatibility в реальных сервисах.
- [ ] Практика: подключить `grpcurl` к своему серверу, включить reflection.

## День 10: Итоговый проект

- [ ]  Собрать небольшой сервис целиком: .proto-схема с 2-3 сообщениями и сервисом с unary + streaming методами, C++ сервер и клиент, CMake-сборка через vcpkg, базовая обработка ошибок и TLS (опционально).

## Ресурсы

- Официальная документация: protobuf.dev (Language Guide, C++ Generated Code, C++ API Reference).
- grpc.io — C++ Quickstart и Basics tutorial.
- Исходники примеров в репозиториях `protocolbuffers/protobuf` и `grpc/grpc` (директории `examples/`).
- `google/protobuf/*.proto` в самом пакете protobuf — читать well-known types как эталонные примеры схем.

## Проверка усвоения

После каждого дня — короткая практическая задача (уже встроена в план). В конце: код-ревью своего итогового проекта на день 10 — проверить совместимость схемы, отсутствие утечек памяти (valgrind/asan), корректную обработку ошибок gRPC.