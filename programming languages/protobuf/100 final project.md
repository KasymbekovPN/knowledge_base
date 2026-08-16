---
tags:
  - protobuf
---
[[programming languages/protobuf/_|<=]]

### vcpkg.json
```json
{
    "name": "task-service-demo",
    "version": "1.0.0",
    "builtin-baseline": "a7eda31dc16994fcaa8587982eb833a8695f1b6f",
    "dependencies": [
        {
            "name": "grpc",
            "features": [
                "codegen"
            ]
        },
        {
            "name": "protobuf",
            "features": [
                "libprotoc"
            ]
        },
        "openssl"
    ]
}
```

### CMakeLists.json
```json
{
    "version": 6,
    "configurePresets": [
        {
            "name": "base",
            "hidden": true,
            "generator": "Ninja Multi-Config",
            "binaryDir": "${sourceDir}/build/${presetName}",
            "toolchainFile": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake",
            "cacheVariables": {
                "VCPKG_TARGET_TRIPLET": "x64-windows-static-md",
                "VCPKG_APPLOCAL_DEPS": "OFF",
                "CMAKE_EXPORT_COMPILE_COMMANDS": "ON",
                "CMAKE_CXX_COMPILER": "C:/Program Files/Microsoft Visual Studio/18/Community/VC/Tools/MSVC/14.51.36231/bin/Hostx64/x64/cl.exe",
                "VCPKG_INSTALL_OPTIONS": "--x-buildtrees-root=C:/vb"
            },
            "environment": {
                "INCLUDE": "C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\VC\\Tools\\MSVC\\14.51.36231\\include;C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\VC\\Tools\\MSVC\\14.51.36231\\ATLMFC\\include;C:\\Program Files (x86)\\Windows Kits\\10\\include\\10.0.26100.0\\ucrt;C:\\Program Files (x86)\\Windows Kits\\10\\include\\10.0.26100.0\\um;C:\\Program Files (x86)\\Windows Kits\\10\\include\\10.0.26100.0\\shared;C:\\Program Files (x86)\\Windows Kits\\10\\include\\10.0.26100.0\\winrt;C:\\Program Files (x86)\\Windows Kits\\10\\include\\10.0.26100.0\\cppwinrt",
                "LIB": "C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\VC\\Tools\\MSVC\\14.51.36231\\ATLMFC\\lib\\x64;C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\VC\\Tools\\MSVC\\14.51.36231\\lib\\x64;C:\\Program Files (x86)\\Windows Kits\\10\\lib\\10.0.26100.0\\ucrt\\x64;C:\\Program Files (x86)\\Windows Kits\\10\\lib\\10.0.26100.0\\um\\x64",
                "PATH": "C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\VC\\Tools\\MSVC\\14.51.36231\\bin\\Hostx64\\x64;C:\\Program Files (x86)\\Windows Kits\\10\\bin\\10.0.26100.0\\x64;$penv{PATH}"
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
            "name": "build-base",
            "hidden": true,
            "environment": {
                "INCLUDE": "C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\VC\\Tools\\MSVC\\14.51.36231\\include;C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\VC\\Tools\\MSVC\\14.51.36231\\ATLMFC\\include;C:\\Program Files (x86)\\Windows Kits\\10\\include\\10.0.26100.0\\ucrt;C:\\Program Files (x86)\\Windows Kits\\10\\include\\10.0.26100.0\\um;C:\\Program Files (x86)\\Windows Kits\\10\\include\\10.0.26100.0\\shared;C:\\Program Files (x86)\\Windows Kits\\10\\include\\10.0.26100.0\\winrt;C:\\Program Files (x86)\\Windows Kits\\10\\include\\10.0.26100.0\\cppwinrt",
                "LIB": "C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\VC\\Tools\\MSVC\\14.51.36231\\ATLMFC\\lib\\x64;C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\VC\\Tools\\MSVC\\14.51.36231\\lib\\x64;C:\\Program Files (x86)\\Windows Kits\\10\\lib\\10.0.26100.0\\ucrt\\x64;C:\\Program Files (x86)\\Windows Kits\\10\\lib\\10.0.26100.0\\um\\x64",
                "PATH": "C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\VC\\Tools\\MSVC\\14.51.36231\\bin\\Hostx64\\x64;C:\\Program Files (x86)\\Windows Kits\\10\\bin\\10.0.26100.0\\x64;$penv{PATH}"
            }
        },
        {
            "name": "debug",
            "inherits": "build-base",
            "configurePreset": "debug",
            "configuration": "Debug"
        },
        {
            "name": "release",
            "inherits": "build-base",
            "configurePreset": "release",
            "configuration": "Release"
        }
    ]
}
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 4.4.2)
project(task_service_demo CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

# ---------- task_service.proto ----------
set(TASK_PROTO ${CMAKE_CURRENT_SOURCE_DIR}/task_service.proto)
set(TASK_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated_task)
file(MAKE_DIRECTORY ${TASK_GEN_DIR})

set(TASK_GENERATED_SRCS
        ${TASK_GEN_DIR}/task_service.pb.cc ${TASK_GEN_DIR}/task_service.pb.h
        ${TASK_GEN_DIR}/task_service.grpc.pb.cc ${TASK_GEN_DIR}/task_service.grpc.pb.h
)

add_custom_command(
        OUTPUT ${TASK_GENERATED_SRCS}
        COMMAND protobuf::protoc
        ARGS --cpp_out=${TASK_GEN_DIR}
        --grpc_out=${TASK_GEN_DIR}
        --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
        -I ${CMAKE_CURRENT_SOURCE_DIR}
        ${TASK_PROTO}
        DEPENDS ${TASK_PROTO} protobuf::protoc gRPC::grpc_cpp_plugin
        COMMENT "Generation task_service.pb.* / .grpc.pb.*"
        VERBATIM
)

add_library(task_proto_grpc OBJECT ${TASK_GENERATED_SRCS})
target_include_directories(task_proto_grpc PUBLIC ${TASK_GEN_DIR})
target_link_libraries(task_proto_grpc PUBLIC protobuf::libprotobuf gRPC::grpc++)

# ---------- Сервер: реализация + reflection ----------
add_executable(task_server task_server.cpp)
target_link_libraries(task_server PRIVATE
        task_proto_grpc
        # включает InitProtoReflectionServerBuilderPlugin()
        gRPC::grpc++_reflection
)

# ---------- Клиент ----------
add_executable(task_client task_client.cpp)
target_link_libraries(task_client PRIVATE task_proto_grpc)

# ---------- Сертификаты для TLS/mTLS ----------
# В демо пути к сертификатам зашиты как /tmp/certs/*.pem в .cc файлах для
# простоты песочницы; в реальном проекте лучше сделать их настраиваемыми
# через аргументы командной строки или переменные окружения, а не hardcode.
message(STATUS "Do not forget to generate certs/{ca,server,client}.{key,crt}")
```

### task_service.proto
```protobuf
syntax = "proto3";
package myapp.tasks.v1;

// === Messages ===

message Task {
  int32 id = 1;
  string title = 2;
  string description = 3;
  bool done = 4;
}

message CreateTaskRequest {
  string title = 1;
  string description = 2;
}

message TaskId {
  int32 id = 1;
}

message ListTasksRequest {}

message BulkCreateSummary {
  int32 created_count = 1;
}

// === Service ===

service TaskService {
  // Unary: создать одну задачу
  rpc CreateTask(CreateTaskRequest) returns (Task);

  // Unary: получить задачу по id. Возвращает NOT_FOUND, если задачи нет —
  // проверка обработки ошибок в клиенте.
  rpc GetTask(TaskId) returns (Task);

  // Server streaming: отдать все задачи по одной, по мере готовности.
  rpc ListTasks(ListTasksRequest) returns (stream Task);

  // Client streaming: массовое создание — клиент шлёт много задач подряд,
  // сервер отвечает один раз со сводкой в конце.
  rpc BulkCreateTasks(stream CreateTaskRequest) returns (BulkCreateSummary);
}
```

### params.hpp
```cpp
#pragma once

#include <string>
#include <unordered_set>

namespace params {
    const std::string ADDRESS{"127.0.0.1:5000"};

    const std::string MODE_INSECURE{"insecure"};
    const std::string MODE_TLS{"tls"};
    const std::string MODE_MTLS{"mtls"};
    const std::string MODE_DEFAULT{MODE_INSECURE};

    const std::unordered_set<std::string> MODES{MODE_INSECURE, MODE_TLS, MODE_MTLS};

    inline std::string get_mode(const int argc, char *argv[]) {
        return argc > 1 && MODES.contains(argv[1])
         ? argv[1]
         : MODE_DEFAULT;
    }

    inline bool with_cert(const int argc, char *argv[]) {
        return argc > 2 && std::string(argv[2]) == "with-cert";
    }

}
```

### read_file.hpp
```cpp
#pragma once

#include <string>
#include <fstream>
#include <sstream>

namespace myapp {
    inline std::string read_file(const std::string &path) {
        const std::fstream file(path);
        std::stringstream ss;
        ss << file.rdbuf();

        return ss.str();
    }
}
```

### task_server.cpp
```cpp
#include <iostream>
#include <format>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <grpcpp/grpcpp.h>

#include <grpcpp/ext/proto_server_reflection_plugin.h>

#include "task_service.grpc.pb.h"

#include "params.hpp"
#include "read_file.hpp"

namespace {

    // ===== In-memory хранилище задач =====
    // grpc::Server по умолчанию многопоточный (пул потоков на входящие RPC),
    // поэтому доступ к общему состоянию защищаем мьютексом.
    class TaskStore {
    public:
        myapp::tasks::v1::Task create(const std::string &title,
                                      const std::string& description) {
            std::lock_guard<std::mutex> lock{mutex_};
            myapp::tasks::v1::Task t;
            t.set_id(next_id_++);
            t.set_title(title);
            t.set_description(description);
            t.set_done(false);
            tasks_.push_back(t);

            return t;
        }

        bool get(const int32_t id, myapp::tasks::v1::Task* out) {
            std::lock_guard<std::mutex> lock{mutex_};
            return std::ranges::any_of(tasks_, [&id, &out](const myapp::tasks::v1::Task& t) {
                if (t.id() == id) {
                    *out = t;
                    return true;
                }
                return false;
            });
        }

        std::vector<myapp::tasks::v1::Task> all() {
            std::lock_guard<std::mutex> lock{mutex_};
            return tasks_;
        }

    private:
        std::mutex mutex_;
        std::vector<myapp::tasks::v1::Task> tasks_;
        int32_t next_id_{1};
    };

    class TaskServiceImpl final: public myapp::tasks::v1::TaskService::Service {
    public:
        grpc::Status CreateTask(grpc::ServerContext *context, const myapp::tasks::v1::CreateTaskRequest *request,
            myapp::tasks::v1::Task *response) override {
            if (request->title().empty()) {
                // Базовая обработка ошибок: невалидный ввод -> INVALID_ARGUMENT,
                // а не молчаливое создание "пустой" задачи.
                return grpc::Status{grpc::StatusCode::INVALID_ARGUMENT, "title must not by empty"};
            }

            *response = store_.create(request->title(), request->description());
            std::cout << std::format("[server] CreateTask -> id= {}, '{}'\n",
                response->id(), response->title());

            return grpc::Status::OK;
        }

        grpc::Status GetTask(grpc::ServerContext *context, const myapp::tasks::v1::TaskId *request,
            myapp::tasks::v1::Task *response) override {
            if (store_.get(request->id(), response)) {
                std::cout << std::format("[server] GetTask({}) -> found\n", request->id());

                return grpc::Status::OK;
            }

            std::cout << std::format("[server] GetTask({}) -> not found\n", request->id());
            return grpc::Status{
                grpc::StatusCode::NOT_FOUND,
                std::format("task {} not found", request->id())
            };
        }

        grpc::Status ListTasks(grpc::ServerContext *context, const myapp::tasks::v1::ListTasksRequest *request,
            grpc::ServerWriter<myapp::tasks::v1::Task> *writer) override {
            const auto tasks{store_.all()};
            std::cout << std::format("[server] ListTasks -> streaming {} tasks\n", tasks.size());
            for (const auto& t: tasks) writer->Write(t);

            return grpc::Status::OK;
        }

        grpc::Status BulkCreateTasks(grpc::ServerContext *context,
            grpc::ServerReader<myapp::tasks::v1::CreateTaskRequest> *reader,
            myapp::tasks::v1::BulkCreateSummary *summary) override {

            myapp::tasks::v1::CreateTaskRequest request;
            int count{};
            while (reader->Read(&request)) {
                if (!request.title().empty()) {
                    store_.create(request.title(), request.description());
                    ++count;
                }
            }

            summary->set_created_count(count);
            std::cout << std::format("[server] BulkCreateTask -> created {} tasks\n", count);

            return grpc::Status::OK;
        }
    private:
        TaskStore store_;
    };

}

int main(const int argc, char *argv[]) {
    const std::string mode{params::get_mode(argc, argv)};

    TaskServiceImpl service;
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();

    grpc::ServerBuilder builder;

    std::shared_ptr<grpc::ServerCredentials> creds;
    if (mode == params::MODE_INSECURE) {
        creds = grpc::InsecureServerCredentials();
    } else {
        grpc::SslServerCredentialsOptions ssl_opts{
            mode == params::MODE_MTLS
            ? GRPC_SSL_REQUEST_AND_REQUIRE_CLIENT_CERTIFICATE_AND_VERIFY
            : GRPC_SSL_DONT_REQUEST_CLIENT_CERTIFICATE
        };
        grpc::SslServerCredentialsOptions::PemKeyCertPair cert_pair;
        cert_pair.private_key = myapp::read_file("certs/server.key");
        cert_pair.cert_chain = myapp::read_file("certs/server.crt");
        ssl_opts.pem_key_cert_pairs.push_back(cert_pair);
        if (mode == params::MODE_MTLS) {
            ssl_opts.pem_root_certs = myapp::read_file("certs/ca.crt");
        }
        creds = grpc::SslServerCredentials(ssl_opts);
    }

    builder.AddListeningPort(params::ADDRESS, creds);
    builder.RegisterService(&service);

    const std::unique_ptr<grpc::Server> server{builder.BuildAndStart()};
    std::cout << std::format("[server] listen to {}, mode= {}, reflection ON\n", params::ADDRESS, mode);
    server->Wait();

    return 0;
}
```

### task_client.cpp
```cpp
#include <iostream>
#include <format>
#include <memory>

#include <grpcpp/grpcpp.h>

#include "task_service.grpc.pb.h"

#include "params.hpp"
#include "read_file.hpp"

namespace {
    void demoCreateTask(myapp::tasks::v1::TaskService::Stub* stub,
                        const std::string& title) {
        grpc::ClientContext context;
        myapp::tasks::v1::CreateTaskRequest request;
        request.set_title(title);
        request.set_description(std::format("description for {}\n", title));
        myapp::tasks::v1::Task response;

        const auto status{stub->CreateTask(&context, request, &response)};
        std::cout << std::format("[client] CreateTask('{}') -> ok= {}", title, status.ok());
        if (status.ok()) std::cout << std::format(", id= {}", response.id());
        std::cout << '\n';
    }

    void demoGetTask(myapp::tasks::v1::TaskService::Stub* stub, const int id) {
        grpc::ClientContext context;
        myapp::tasks::v1::TaskId request;
        request.set_id(id);
        myapp::tasks::v1::Task response;

        const auto status{stub->GetTask(&context, request, &response)};
        std::cout << std::format("[client] GetTask('{}') -> ok= {}", id, status.ok());
        if (status.ok()) {
            std::cout << std::format(", title = '{}'", response.title());
        } else {
            // Обработка ошибки: различаем код, а не просто "что-то пошло не так"
            std::cout << std::format(", code= {}, ({}), message= {}",
                static_cast<int>(status.error_code()),
                (status.error_code() == grpc::StatusCode::NOT_FOUND
                    ? "NOT FOUND"
                    : "OTHER"),
                status.error_message());
        }
        std::cout << '\n';
    }

    void demoListTasks(myapp::tasks::v1::TaskService::Stub* stub) {
        grpc::ClientContext context;
        const myapp::tasks::v1::ListTasksRequest request;
        std::unique_ptr<grpc::ClientReader<myapp::tasks::v1::Task>> reader{
            stub->ListTasks(&context, request)
        };

        std::cout << "[client] ListTasks(server streaming):";
        myapp::tasks::v1::Task task;
        while (reader->Read(&task)) {
            std::cout << std::format(" <- id= {}, '{}'\n", task.id(), task.title());
        }

        std::cout << std::format("[client] ListTasks completed, ok= {}\n", reader->Finish().ok());
    }

    void demoBulkCreateTasks(myapp::tasks::v1::TaskService::Stub* stub) {
        grpc::ClientContext context;
        myapp::tasks::v1::BulkCreateSummary summary;
        const std::unique_ptr<grpc::ClientWriter<myapp::tasks::v1::CreateTaskRequest>> writer{
            stub->BulkCreateTasks(&context, &summary)
        };

        std::cout << std::format("[client] BulkCreateTasks (client streaming):\n");
        for (const std::string& title : {"bulk-1", "bulk-2", "bulk-3"}) {
            myapp::tasks::v1::CreateTaskRequest request;
            request.set_title(title);
            writer->Write(request);
            std::cout << std::format("  -> sent: '{}'\n", title);
        }

        writer->WritesDone();
        std::cout << std::format("[client] BulkCreateTasks, ok= {}, created= {}\n",
            writer->Finish().ok(),
            summary.created_count());
    }

}

int main(const int argc, char *argv[]) {
    const std::string mode{params::get_mode(argc, argv)};
    const bool send_client_cert{params::with_cert(argc, argv)};

    std::shared_ptr<grpc::ChannelCredentials> creds;
    grpc::ChannelArguments args;

    if (mode == params::MODE_INSECURE) {
        creds = grpc::InsecureChannelCredentials();
    } else {
        grpc::SslCredentialsOptions ssl_opts;
        ssl_opts.pem_root_certs = myapp::read_file("certs/ca.crt");
        if (mode == params::MODE_MTLS && send_client_cert) {
            ssl_opts.pem_private_key = myapp::read_file("certs/client.key");
            ssl_opts.pem_cert_chain = myapp::read_file("certs/client.crt");
        }
        args.SetSslTargetNameOverride("localhost");
        creds = grpc::SslCredentials(ssl_opts);
    }

    const auto channel{grpc::CreateCustomChannel(params::ADDRESS, creds, args)};
    std::unique_ptr<myapp::tasks::v1::TaskService::Stub> stub{
        myapp::tasks::v1::TaskService::NewStub(channel)
    };

    std::cout << std::format("=== mode = {} ===\n",
        (send_client_cert ? "CRT" : "NO CRT"));

    demoCreateTask(stub.get(), "write demo");
    demoCreateTask(stub.get(), "make tests");
    demoGetTask(stub.get(), 1);
    demoGetTask(stub.get(), 999);
    demoBulkCreateTasks(stub.get());
    demoListTasks(stub.get());

    return 0;
}

```

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
- [x] Bidi streaming. (2026.08.11)
- [x] Асинхронный API (`CompletionQueue`) — на уровне понимания, без глубокого погружения. (2026.08.11)
- [x] Обработка ошибок: `grpc::Status`, коды ошибок, deadlines/timeouts, retry-политики. (2026.08.11)
- [x] Практика: добавить server-streaming метод. (2026.08.11)

## День 9: Продвинутые темы

- [x] Interceptors в gRPC (аутентификация, логирование). (2026.08.12)
- [x] TLS/mTLS для защищённых соединений. (2026.08.15)
- [x] Reflection service и `grpcurl` для отладки без клиента. (2026.08.15)
- [x] Версионирование API и backward-compatibility в реальных сервисах. (2026.08.15)

## День 10: Итоговый проект

- [x]  Собрать небольшой сервис целиком: .proto-схема с 2-3 сообщениями и сервисом с unary + streaming методами, C++ сервер и клиент, CMake-сборка через vcpkg, базовая обработка ошибок и TLS + mtls, reflection (2026.08.15)

## Ресурсы

- Официальная документация: protobuf.dev (Language Guide, C++ Generated Code, C++ API Reference).
- grpc.io — C++ Quickstart и Basics tutorial.
- Исходники примеров в репозиториях `protocolbuffers/protobuf` и `grpc/grpc` (директории `examples/`).
- `google/protobuf/*.proto` в самом пакете protobuf — читать well-known types как эталонные примеры схем.

## Проверка усвоения

После каждого дня — короткая практическая задача (уже встроена в план). В конце: код-ревью своего итогового проекта на день 10 — проверить совместимость схемы, отсутствие утечек памяти (valgrind/asan), корректную обработку ошибок gRPC.