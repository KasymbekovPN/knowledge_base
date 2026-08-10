---
tags:
  - protobuf
---
[[programming languages/protobuf/_|<=]]

### vcpkg.json
```json
{
  "name": "grpc-cpp-demo",
  "version": "1.0.0",
  "dependencies": [
    "grpc",
    "protobuf"
  ]
}
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 4.4.2)
project(grpc_log_service CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# С vcpkg (см. vcpkg.json — dependencies: grpc, protobuf) оба пакета
# ставят свои CMake config-файлы, поэтому используем CONFIG-режим,
# а не module-режим (find_package(Protobuf) без CONFIG).
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

set(PROTO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/log_service.proto)
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_DIR})

set(GENERATED_SRCS
        ${GENERATED_DIR}/log_service.pb.cc
        ${GENERATED_DIR}/log_service.pb.h
        ${GENERATED_DIR}/log_service.grpc.pb.cc
        ${GENERATED_DIR}/log_service.grpc.pb.h
)

# protobuf_generate() из FindProtobuf.cmake не умеет одновременно cpp_out +
# grpc_out с внешним плагином, поэтому кодогенерацию делаем вручную —
# ровно то же самое мы уже проверяли на связке FetchContent + protobuf::protoc.
add_custom_command(
        OUTPUT ${GENERATED_SRCS}
        COMMAND protobuf::protoc
        ARGS --cpp_out=${GENERATED_DIR}
        --grpc_out=${GENERATED_DIR}
        --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
        -I ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROTO_FILE}
        DEPENDS ${PROTO_FILE} protobuf::protoc gRPC::grpc_cpp_plugin
        COMMENT "Generation *.pb.* and *.grpc.pb.* from order_service.proto"
        VERBATIM
)

add_library(log_proto_grpc OBJECT ${GENERATED_SRCS})
target_include_directories(log_proto_grpc PUBLIC ${GENERATED_DIR})
target_link_libraries(log_proto_grpc PUBLIC protobuf::libprotobuf gRPC::grpc++)

add_executable(demo_grpc_sync main.cpp)
target_link_libraries(demo_grpc_sync PRIVATE log_proto_grpc)

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

### log_service.proto
```Protobuf
syntax = "proto3";
package myapp;

message LogQuery {
  string service_name = 1;
}

message LogEntry {
  string level = 1;
  string message = 2;
}

message UploadSummary {
  int32 accepted_count = 1;
  string status = 2;
}

service LogService {
  // Server streaming: клиент один раз просит логи конкретного сервиса,
  // сервер "живьём" отдаёт их по мере появления.
  rpc StreamLogs(LogQuery) returns (stream LogEntry);

  // Client streaming: клиент шлёт логи пачкой (по одному сообщению за раз),
  // сервер копит и в конце присылает сводку.
  rpc UploadLogs(stream LogEntry) returns (UploadSummary);
}

```

### main.cpp
```cpp
#include <chrono>
#include <iostream>
#include <format>
#include <memory>
#include <thread>

#include <grpcpp/grpcpp.h>

#include "log_service.grpc.pb.h"

namespace {
    // ### SERVER ###

    std::string now_ms() {
        const auto now{std::chrono::system_clock::now().time_since_epoch()};
        const auto ms{std::chrono::duration_cast<std::chrono::milliseconds>(now).count()};

        return std::to_string(ms % 1'000'000);
    }

    class LogServiceImpl final: public myapp::LogService::Service {
        // ===== Server streaming: сервер САМ решает, когда писать в поток =====
        grpc::Status StreamLogs(grpc::ServerContext* context,
                                const myapp::LogQuery* request,
                                grpc::ServerWriter<myapp::LogEntry>* writer) override {
            std::cout << std::format("[t= {}] [server] log streaming start for '{}'\n",
                now_ms(),
                request->service_name());
            const char* messages[] = {
                "service started",
                "connected to db",
                "handling request #1",
                "handling request #2",
                "graceful shutdown"
            };

            for (const char* msg: messages) {
                // имитируем, что лог реально появляется "по мере поступления",
                // а не берётся из уже готового списка мгновенно
                std::this_thread::sleep_for(std::chrono::milliseconds(400));

                myapp::LogEntry entry;
                entry.set_level("INFO");
                entry.set_message(msg);
                std::cout << std::format("[t= {}] [server] -> write into stream '{}'\n", now_ms(), msg);
                writer->Write(entry);
            }

            std::cout << std::format("[t= {}] [server] log stream done\n", now_ms());

            return grpc::Status::OK;
        }

        // ===== Client streaming: сервер читает, пока клиент не закончит =====
        grpc::Status UploadLogs(grpc::ServerContext* context,
                                grpc::ServerReader<myapp::LogEntry>* reader,
                                myapp::UploadSummary* summary) override {
            myapp::LogEntry entry;
            int count{};
            while (reader->Read(&entry)) {
                ++count;
                std::cout << std::format("[t= {}] [server] <- take log #{}: [{}] {}\n",
                    now_ms(),
                    count,
                    entry.level(),
                    entry.message());
            }

            summary->set_accepted_count(count);
            summary->set_status("OK");
            std::cout << std::format("[t= {}] [server] loading done, taken {} entry\n", now_ms(), count);

            return grpc::Status::OK;
        }
    };

    void start_server() {
        std::string address{"127.0.0.1:5081"};
        LogServiceImpl service;

        grpc::ServerBuilder builder;
        builder.AddListeningPort(address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);

        std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
        std::cout << std::format("[server] listen to {}\n", address);
        server->Wait();
    }

    // ### CLIENT ###

    // ===== Server streaming: клиент читает по мере поступления, НЕ ждёт всё сразу =====
    void demoStreamLogs(myapp::LogService::Stub* stub) {
        std::cout << "\n=== Server streaming: StreamLogs ===\n";
        grpc::ClientContext context;
        myapp::LogQuery query;
        query.set_service_name("payments");

        std::unique_ptr<grpc::ClientReader<myapp::LogEntry>> reader{
            stub->StreamLogs(&context, query)
        };

        myapp::LogEntry entry;
        while (reader->Read(&entry)) {
            // именно этот момент времени доказывает, что данные пришли ПОСТЕПЕННО,
            // а не все разом в конце вызова
            std::cout << std::format("[t= {}] [client] log received: [{}] '{}'\n",
                now_ms(),
                entry.level(),
                entry.message());
        }

        std::cout << std::format("[client] StreamLogs done, ok= {}\n", reader->Finish().ok());
    }

    // ===== Client streaming: клиент сам решает темп отправки =====
    void demoUploadLogs(myapp::LogService::Stub* stub) {
        std::cout << "\n=== Client streaming: UploadLogs ===\n";
        grpc::ClientContext context;
        myapp::UploadSummary summary;

        std::unique_ptr<grpc::ClientWriter<myapp::LogEntry>> writer{
            stub->UploadLogs(&context, &summary)
        };

        const char* messages[] = {
            "cache miss",
            "retrying request",
            "recovered"
        };
        for (const char* msg: messages) {
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            myapp::LogEntry entry;
            entry.set_level("WARN");
            entry.set_message(msg);
            std::cout << std::format("[t= {}] [client] -> sending: '{}'\n", now_ms(), msg);
            writer->Write(entry);
        }

        writer->WritesDone();
        grpc::Status status = writer->Finish();
        std::cout << std::format("[client] UploadLogs done, ok= {}, received= {}, status={}\n",
            status.ok(),
            summary.accepted_count(),
            summary.status());
    }

    void start_client() {
        std::string address{"127.0.0.1:5081"};
        auto channel = grpc::CreateChannel(address, grpc::InsecureChannelCredentials());
        std::unique_ptr<myapp::LogService::Stub> stub = myapp::LogService::NewStub(channel);

        demoStreamLogs(stub.get());
        demoUploadLogs(stub.get());
    }
}

int main(const int argc, char *argv[]) {
    if (const std::string START_KIND{argc > 1 ? argv[1] : ""};
        START_KIND == "server") start_server();
    else if (START_KIND == "client") start_client();
    else std::cout << "BAD KIND\n";

    return 0;
}

```

---

**Server streaming (`StreamLogs`) — таймлайн**

```
[t=44139] сервер начал стримить
[t=44539] сервер -> пишет: service started       [t=44540] клиент <- получил (задержка ~1мс!)
[t=44940] сервер -> пишет: connected to db        [t=44941] клиент <- получил
[t=45341] сервер -> пишет: handling request #1    [t=45341] клиент <- получил
[t=45741] сервер -> пишет: handling request #2    [t=45741] клиент <- получил
[t=46141] сервер -> пишет: graceful shutdown       [t=46142] клиент <- получил
```

Между записями сервер намеренно ждёт по 400 мс (`sleep_for` в коде, имитирует "логи появляются по мере событий"). Клиент получает **каждую** запись почти мгновенно после того, как сервер её написал — не ждёт полного завершения стрима. Это принципиально отличается от того, что было бы, если бы сервер сначала собрал всё в `repeated LogEntry`, а потом отдал одним unary-ответом: тогда клиент увидел бы все 5 записей одновременно, только спустя ~2 секунды (400×5) — а не постепенно.

Код сервера — просто цикл с `writer->Write(entry)` на каждой итерации, gRPC сам занимается доставкой каждого сообщения по мере вызова `Write`, не дожидаясь `return`.

**Client streaming (`UploadLogs`) — таймлайн**

```
[t=46442] клиент -> отправляет: cache miss         [t=46442] сервер <- принял #1
[t=46742] клиент -> отправляет: retrying request    [t=46743] сервер <- принял #2
[t=47043] клиент -> отправляет: recovered            [t=47043] сервер <- принял #3
[t=47043] сервер: загрузка завершена, принято 3
```

Здесь наоборот — клиент сам решает темп отправки (300 мс между сообщениями), сервер вычитывает их по мере поступления через `reader->Read()` в цикле, и только после `writer->WritesDone()` от клиента сервер отдаёт единственный `UploadSummary`. Практический сценарий — именно "стрим логов на сервер": агент на машине шлёт логи по мере появления, не дожидаясь буферизации в один большой батч.

**Ключевое отличие от вызова unary N раз подряд**

И server streaming, и client streaming работают в рамках **одного** HTTP/2-соединения/потока (`ClientContext` создан один раз на весь стрим) — в отличие от N отдельных unary-вызовов, здесь не нужно N раз проходить полный overhead установления вызова (handshake на уровне gRPC-стрима, метаданные), да и порядок сообщений строго гарантирован в рамках потока.


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

- [x] `find_package(Protobuf REQUIRED)`, `protobuf_generate_cpp()`. (2026.08.09)
- [x] Альтернатива: `FetchContent`/vcpkg для protobuf как зависимости. (2026.08.09)
- [x] Организация .proto файлов в отдельной директории, генерация в build-директорию, инкрементальная пересборка. (2026.08.09)

## День 6-7: gRPC — основы

- [x] Установка `grpc` и `grpc_cpp_plugin`. (2026.08.09)
- [x] Синтаксис сервисов в .proto: `service`, `rpc`, четыре типа вызовов (unary, server streaming, client streaming, bidi streaming). (2026.08.09)
- [x] Генерация: `--grpc_out` и `--plugin=protoc-gen-grpc-cpp`. (2026.08.09)
- [x] Синхронный сервер/клиент на C++: `grpc::Server`, `ServerBuilder`, `ClientContext`, `Stub`. (2026.08.10)
- [x] Практика: реализовать unary RPC (например, `GetUser(UserRequest) -> UserResponse`), поднять сервер и клиент локально. (2026.08.10)

## День 8: gRPC — streaming и асинхронность

- [x] Server streaming и client streaming на практике (например, стрим логов). (2026.08.10)
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