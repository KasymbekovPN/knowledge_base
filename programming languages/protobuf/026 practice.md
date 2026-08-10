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
project(grpc_user_service CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# С vcpkg (см. vcpkg.json — dependencies: grpc, protobuf) оба пакета
# ставят свои CMake config-файлы, поэтому используем CONFIG-режим,
# а не module-режим (find_package(Protobuf) без CONFIG).
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

set(PROTO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/user_service.proto)
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_DIR})

set(GENERATED_SRCS
        ${GENERATED_DIR}/user_service.pb.cc
        ${GENERATED_DIR}/user_service.pb.h
        ${GENERATED_DIR}/user_service.grpc.pb.cc
        ${GENERATED_DIR}/user_service.grpc.pb.h
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

add_library(user_proto_grpc OBJECT ${GENERATED_SRCS})
target_include_directories(user_proto_grpc PUBLIC ${GENERATED_DIR})
target_link_libraries(user_proto_grpc PUBLIC protobuf::libprotobuf gRPC::grpc++)

add_executable(demo_grpc_sync main.cpp)
target_link_libraries(demo_grpc_sync PRIVATE user_proto_grpc)

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

### user_service.proto
```Protobuf
syntax = "proto3";
package myapp;

message UserRequest {
  int32 user_id = 1;
}

message UserResponse {
  int32 user_id = 1;
  string name = 2;
  string email = 3;
}

service UserService {
  rpc GetUser(UserRequest) returns (UserResponse);
}

```

### main.cpp
```cpp
#include <iostream>
#include <format>
#include <string>
#include <memory>

#include <grpcpp/grpcpp.h>

#include "user_service.grpc.pb.h"


namespace {
    // ### SERVER ###

    // Простейшая "база данных" в памяти — для примера этого достаточно.
    class UserServiceImpl final : public myapp::UserService::Service {
        grpc::Status GetUser(grpc::ServerContext* context,
                             const myapp::UserRequest* request,
                             myapp::UserResponse* response) override {

            std::cout << std::format("[server] requesr GetUser(user_id={})\n", request->user_id());

            if (request->user_id() == 1) {
                response->set_user_id(1);
                response->set_name("Alice");
                response->set_email("alice@example.com");

                return grpc::Status::OK;
            }

            // Пользователь не найден — возвращаем ошибку через grpc::Status,
            // а не через "пустое" сообщение. Клиент должен проверять status.ok().
            return grpc::Status{grpc::StatusCode::NOT_FOUND, "user not found"};
        }
    };

    void start_server() {
        std::string address{"127.0.0.1:50070"};
        UserServiceImpl service;

        grpc::ServerBuilder builder;
        builder.AddListeningPort(address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);

        std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
        std::cout << std::format("[server] listening to {}, wait request...", address);
        // блокирует процесс — это отдельный, самостоятельный сервер
        server->Wait();
    }

    // ### CLIENT ###

    void callGetUser(myapp::UserService::Stub* stub, const int user_id) {
        myapp::UserRequest request;
        request.set_user_id(user_id);

        myapp::UserResponse response;
        grpc::ClientContext context;
        grpc::Status status{
            stub->GetUser(&context, request, &response)
        };

        if (status.ok()) {
            std::cout << std::format("[client] GetUser({}) -> {} <{}>\n",
                user_id,
                response.name(),
                response.email());
        } else {
            std::cout
                << std::format("[client] GetUser({}) -> error ", user_id)
                << status.error_code() << " "
                << status.error_message()
                << std::endl;
        }
    }

    void start_client() {
        std::string address("127.0.0.1:50070");
        auto channel = grpc::CreateChannel(address, grpc::InsecureChannelCredentials());
        std::unique_ptr<myapp::UserService::Stub> stub = myapp::UserService::NewStub(channel);

        callGetUser(stub.get(), 1);   // существующий пользователь
        callGetUser(stub.get(), 999); // несуществующий — проверяем обработку ошибки
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


**`server.cc` — реализация**

```cpp
class UserServiceImpl final : public UserService::Service {
  Status GetUser(ServerContext* context, const UserRequest* request,
                 UserResponse* response) override {
    if (request->user_id() == 1) {
      response->set_name("Alice");
      ...
      return Status::OK;
    }
    return Status(grpc::StatusCode::NOT_FOUND, "user not found");
  }
};
```

Лог сервера после обращений клиента:

```
[сервер] слушает на 127.0.0.1:50070, ждёт запросов...
[сервер] запрос GetUser(user_id=1)
[сервер] запрос GetUser(user_id=999)
```

Обработал оба вызова — существующего пользователя (id=1) и несуществующего (id=999), вернув в последнем случае `grpc::Status` с кодом `NOT_FOUND`, а не пустой/дефолтный `UserResponse`. Это важный паттерн: ошибки в gRPC передаются через `Status`, а не через содержимое сообщения-ответа.

**`client.cc` — вызов**

```cpp
auto channel = grpc::CreateChannel(address, grpc::InsecureChannelCredentials());
auto stub = UserService::NewStub(channel);

Status status = stub->GetUser(&context, request, &response);
if (status.ok()) { ... } else { ... }
```

Вывод клиента:

```
[клиент] GetUser(1) -> Alice <alice@example.com>
[клиент] GetUser(999) -> ОШИБКА: 5 user not found
```

Код `5` — это numeric-значение `grpc::StatusCode::NOT_FOUND` (перечисление кодов совпадает с HTTP/2-независимой gRPC-спецификацией статусов). Клиент корректно различил успех и ошибку через `status.ok()`, не полагаясь на догадки по содержимому ответа.

**Ловушка, которую поймал по пути**

Первый запуск дал пустой лог сервера, хотя данные пришли клиенту верно — `kill` оборвал процесс раньше, чем буферизованный `std::cout` сбросился на диск/терминал. Пришлось перезапустить с `stdbuf -oL` (построчная буферизация) для вывода. В реальном коде для сервера обычно либо явно `std::cout << ... << std::flush`, либо (что правильнее) — нормальная система логирования (`glog`, `spdlog`), а не сырой `std::cout`.
