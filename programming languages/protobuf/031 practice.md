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
        COMMENT "Generation *.pb.* and *.grpc.pb.* from user_service.proto"
        VERBATIM
)

add_library(user_proto_grpc OBJECT ${GENERATED_SRCS})
target_include_directories(user_proto_grpc PUBLIC ${GENERATED_DIR})
target_link_libraries(user_proto_grpc PUBLIC protobuf::libprotobuf gRPC::grpc++)

add_executable(demo main.cpp)
target_link_libraries(demo PRIVATE user_proto_grpc)

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
                "VCPKG_TARGET_TRIPLET": "x64-windows-static-md",
                "VCPKG_APPLOCAL_DEPS": "OFF"
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
```protobuf
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

message ListUsersRequest {}

service UserService {
  rpc GetUser(UserRequest) returns (UserResponse);

  // Новый server-streaming метод: один запрос — сервер отдаёт всех
  // пользователей по одному, по мере готовности, а не единым списком.
  rpc ListUsers(ListUsersRequest) returns (stream UserResponse);
}

```

### main.cpp
```cpp
#include <chrono>
#include <iostream>
#include <format>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <grpcpp/grpcpp.h>

#include "user_service.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;
using myapp::ListUsersRequest;
using myapp::UserRequest;
using myapp::UserResponse;
using myapp::UserService;
using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;

namespace {
    const std::string ADDRESS{"127.0.0.1:5000"};

    // ### SERVER ###

    // Простейшая "база данных" в памяти — для примера этого достаточно.
    struct StoredUser {
        int id;
        std::string name;
        std::string email;
    };

    const std::vector<StoredUser> K_USERS {
        {.id = 1, .name = "Alice", .email = "alice@example.com"},
        {.id = 2, .name = "Bob", .email = "bob@example.com"},
        {.id = 3, .name = "Carol", .email = "carol@example.com"},
    };

    class UserServiceImpl final : public UserService::Service {
        Status GetUser(ServerContext* context, const UserRequest* request, UserResponse* response) override {
            const auto user_id{request->user_id()};
            std::cout << std::format("[server] request GetUser(user_id= {})\n", user_id);

            for (const auto&[id, name, email]: K_USERS) {
                if (id == user_id) {
                    response->set_user_id(id);
                    response->set_name(name);
                    response->set_email(email);

                    return Status::OK;
                }
            }

            // Пользователь не найден — возвращаем ошибку через grpc::Status,
            // а не через "пустое" сообщение. Клиент должен проверять status.ok().

            return Status(grpc::StatusCode::NOT_FOUND, "user not found");
        }

        // Server streaming: пишем пользователей в поток по одному, с задержкой —
        // чтобы было видно, что они приходят клиенту постепенно, а не одним куском.
        Status ListUsers(ServerContext* context, const ListUsersRequest*, ServerWriter<UserResponse>* writer) override {
            std::cout << std::format("[server] request ListUsers, will start streaming {} users\n", K_USERS.size());

            for (const auto& [id, name, email]: K_USERS) {
                std::this_thread::sleep_for(std::chrono::milliseconds(300));

                UserResponse response;
                response.set_user_id(id);
                response.set_name(name);
                response.set_email(email);

                std::cout << std::format("[server] -> write to stream: {}\n", name);
                writer->Write(response);
            }

            std::cout << "[server] ListUsers completed\n";
            return Status::OK;
        }
    };

    void start_server() {
        UserServiceImpl service;

        ServerBuilder builder;
        builder.AddListeningPort(ADDRESS, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);

        std::unique_ptr<Server> server(builder.BuildAndStart());
        std::cout << std::format("[server] listens to {}, waits requests...\n", ADDRESS);
        // блокирует процесс — это отдельный, самостоятельный сервер
        server->Wait();
    }

    // ### CLIENT ###

    void callListUsers(UserService::Stub* stub) {
        std::cout << "\n[client] calls ListUsers (server streaming)\n";
        ClientContext context;
        ListUsersRequest request;

        std::unique_ptr<ClientReader<UserResponse>> reader{
            stub->ListUsers(&context, request)
        };

        UserResponse response;
        while (reader->Read(&response)) {
            std::cout << std::format("[client] <- gotten user: id= {}, {}, <{}>\n",
                response.user_id(), response.name(), response.email());
        }

        std::cout << std::format("[client] ListUsers completed, ok= {}\n", reader->Finish().ok());
    }

    void callGetUser(UserService::Stub* stub, const int user_id) {
        UserRequest request;
        request.set_user_id(user_id);

        UserResponse response;
        ClientContext context;

        Status status{stub->GetUser(&context, request, &response)};

        if (status.ok()) {
            std::cout << std::format("[client] GetUser({}) -> {}, {}\n",
                response.user_id(), response.name(), response.email());
        } else {
            std::cout << std::format("[client] GetUser({}) -> error: {}, {}\n",
                response.user_id(),
                static_cast<int>(status.error_code()),
                status.error_message());
        }
    }

    void start_client() {
        auto channel = grpc::CreateChannel(ADDRESS, grpc::InsecureChannelCredentials());
        std::unique_ptr<UserService::Stub> stub = UserService::NewStub(channel);

        callGetUser(stub.get(), 1);
        callGetUser(stub.get(), 999);
        callListUsers(stub.get());
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


**Что изменилось в `.proto`**

```proto
message ListUsersRequest {}

service UserService {
  rpc GetUser(UserRequest) returns (UserResponse);
  rpc ListUsers(ListUsersRequest) returns (stream UserResponse);  // новое
}
```

Добавление метода в существующий `service` не потребовало трогать `GetUser` — protoc перегенерировал `UserService::Service`/`Stub` с новым методом, не сломав совместимость со старым.

**Сервер**

```cpp
Status ListUsers(ServerContext* context, const ListUsersRequest*,
                  ServerWriter<UserResponse>* writer) override {
  for (const auto& u : kUsers) {
    UserResponse response;
    response.set_user_id(u.id);
    ...
    writer->Write(response);
  }
  return Status::OK;
}
```

"База" расширена до трёх пользователей (`kUsers`), `GetUser` теперь ищет по ней циклом вместо хардкода одного id. Задержка `sleep_for(300ms)` между `Write()` — чтобы наглядно доказать, что данные идут по одному, а не собираются в список и не отдаются разом (тот же приём, что и в демо со стримом логов).

**Клиент**

```cpp
std::unique_ptr<ClientReader<UserResponse>> reader = stub->ListUsers(&context, request);
UserResponse response;
while (reader->Read(&response)) { ... }
Status status = reader->Finish();
```

Ровно тот же паттерн server streaming, что разбирали раньше — `ClientReader`, цикл `Read()`, обязательный `Finish()` в конце.
