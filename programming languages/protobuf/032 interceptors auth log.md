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

### interceptors.hpp
```cpp
#pragma once

#include <chrono>
#include <iostream>
#include <format>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include <grpcpp/support/client_interceptor.h>
#include <grpcpp/support/server_interceptor.h>


static const std::string TOKEN{"Bearer secret-token-123"};
static const std::string KEY{"authorization"};


// ===================== КЛИЕНТСКИЙ ИНТЕРЦЕПТОР =====================
// Добавляет auth-токен в исходящие метаданные КАЖДОГО вызова и логирует
// имя метода + время выполнения — без единой строчки этого кода в
// самих местах вызова stub->Method(...).
class ClientAuthLoggingInterceptor: public grpc::experimental::Interceptor {
public:
    explicit ClientAuthLoggingInterceptor(grpc::experimental::ClientRpcInfo* info):
        info_{info} {}

    void Intercept(grpc::experimental::InterceptorBatchMethods *methods) override {
        if (methods->QueryInterceptionHookPoint(
            grpc::experimental::InterceptionHookPoints::PRE_SEND_INITIAL_METADATA)) {
            start_ = std::chrono::steady_clock::now();
            auto* metadata{methods->GetSendInitialMetadata()};
            metadata->insert({KEY, TOKEN});
            std::cout << std::format("[client-interceptor] -> {}: added auth-token\n", info_->method());
        }

        if (methods->QueryInterceptionHookPoint(
            grpc::experimental::InterceptionHookPoints::POST_RECV_STATUS)) {
            auto elapsed{std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start_).count()};
            grpc::Status* status{methods->GetRecvStatus()};
            std::cout << std::format("[client-interceptor] <- {}: completed in {} ms, ok= {} \n]",
                info_->method(), elapsed, status->ok());
        }

        // обязательно — иначе RPC зависнет
        methods->Proceed();
    }

private:
    grpc::experimental::ClientRpcInfo* info_{nullptr};
    std::chrono::steady_clock::time_point start_;
};

class ClientAuthLoggingInterceptorFactory: public grpc::experimental::ClientInterceptorFactoryInterface {
public:
    grpc::experimental::Interceptor * CreateClientInterceptor(grpc::experimental::ClientRpcInfo *info) override {
        return new ClientAuthLoggingInterceptor(info);
    }
};

// ===================== СЕРВЕРНЫЙ ИНТЕРЦЕПТОР =====================
// Логирует каждый входящий вызов и проверяет auth-токен. Если токена нет
// или он неверный — подменяет исходящий статус на UNAUTHENTICATED.
class ServerAuthLoggingInterceptor: public grpc::experimental::Interceptor {
public:
    explicit ServerAuthLoggingInterceptor(grpc::experimental::ServerRpcInfo *info)
        : info_{info} {
    }

    void Intercept(grpc::experimental::InterceptorBatchMethods *methods) override {
        if (methods->QueryInterceptionHookPoint(
            grpc::experimental::InterceptionHookPoints::POST_RECV_INITIAL_METADATA)) {
            auto* metadata = methods->GetRecvInitialMetadata();
            if (const auto it = metadata->find(KEY);
                it != metadata->end()) {
                token_ = std::string(it->second.data(), it->second.length());
            }
            std::cout << std::format("[server-interceptor] <- {}: token gotten '{}'\n",
                info_->method(),
                (token_.empty() ? "(empty)" : token_));
        }

        if (methods->QueryInterceptionHookPoint(
            grpc::experimental::InterceptionHookPoints::PRE_SEND_STATUS)) {
            if (token_ != TOKEN) {
                std::cout << std::format("[server-interceptor] -> {}: token is invalid/missing,"
                    " change status to UNAUTHENTICATED\n",
                    info_->method());
                methods->ModifySendStatus(grpc::Status{
                    grpc::StatusCode::UNAUTHENTICATED,
                    "invalid or missing token"
                });
            } else {
                std::cout << std::format("[server-interceptor] -> {}: token is valid\n", info_->method());
            }
        }

        // обязательно — иначе RPC зависнет
        methods->Proceed();
    }

private:
    grpc::experimental::ServerRpcInfo *info_{nullptr};
    std::string token_;
};

class ServerAuthLoggingInterceptorFactory: public grpc::experimental::ServerInterceptorFactoryInterface {
public:
    grpc::experimental::Interceptor * CreateServerInterceptor(grpc::experimental::ServerRpcInfo *info) override {
        return new ServerAuthLoggingInterceptor(info);
    }
};

```

### main.cpp
```cpp
#include <iostream>
#include <format>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "user_service.grpc.pb.h"
#include "interceptors.hpp"

static const std::string ADDRESS{"127.0.0.1:5000"};

namespace server_with_interceptor {

    class UserServiceImpl final: public myapp::UserService::Service {
    public:
        grpc::Status GetUser(grpc::ServerContext *context, const myapp::UserRequest *request,
            myapp::UserResponse *response) override {
            // Обрати внимание: сама бизнес-логика ничего не знает про auth —
            // проверка токена целиком вынесена в interceptor.
            std::cout << std::format("[handler] GetUser(user_id= {})\n", request->user_id());
            response->set_user_id(1);
            response->set_name("Alive");
            response->set_email("alice@example.com");

            return grpc::Status::OK;
        }

        grpc::Status ListUsers(grpc::ServerContext *context, const myapp::ListUsersRequest *request,
            grpc::ServerWriter<myapp::UserResponse> *writer) override {
            return grpc::Status::OK;
        }
    };

    void start() {
        UserServiceImpl service;

        grpc::ServerBuilder builder;
        builder.AddListeningPort(ADDRESS, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);

        std::vector<std::unique_ptr<grpc::experimental::ServerInterceptorFactoryInterface>> creators;
        creators.push_back(std::make_unique<ServerAuthLoggingInterceptorFactory>());
        builder.experimental().SetInterceptorCreators(std::move(creators));

        std::unique_ptr<grpc::Server> server{builder.BuildAndStart()};
        std::cout << std::format("[server] listen to {} (with auth-interceptor)\n", ADDRESS);
        server->Wait();
    }
}

namespace client_with_interceptor {

    void start() {
        std::vector<std::unique_ptr<grpc::experimental::ClientInterceptorFactoryInterface>> creators;
        creators.push_back(std::make_unique<ClientAuthLoggingInterceptorFactory>());

        const auto channel{grpc::experimental::CreateCustomChannelWithInterceptors(
            ADDRESS,
            grpc::InsecureChannelCredentials(),
            grpc::ChannelArguments(),
            std::move(creators))};
        const std::unique_ptr<myapp::UserService::Stub> stub{myapp::UserService::NewStub(channel)};

        // Вызов выглядит как обычно — ни намёка на auth-логику здесь нет,
        // весь auth/логирование прилетает "снаружи" через interceptor.
        myapp::UserRequest request;
        request.set_user_id(1);
        myapp::UserResponse response;
        grpc::ClientContext context;

        std::cout << "[client] calling GetUser(...)\n";
        const auto status = stub->GetUser(&context, request, &response);

        std::cout << std::format("[client] status.ok() = {}, ", status.ok());
        if (status.ok()) {
            std::cout << std::format("name= {}", response.name());
        } else {
            std::cout << std::format("error= {}", status.error_message());
        }
        std::cout << '\n';
    }
}

namespace client_without_interceptor {

    void start() {
        // Обычный канал БЕЗ клиентского интерцептора — значит, без auth-токена.
        const auto channel{grpc::CreateChannel(
            ADDRESS,
            grpc::InsecureChannelCredentials())};
        std::unique_ptr<myapp::UserService::Stub> stub{myapp::UserService::NewStub(channel)};

        myapp::UserRequest request;
        request.set_user_id(1);
        myapp::UserResponse response;
        grpc::ClientContext context;

        std::cout << "[client without token] calling GetUser(...)\n";
        const auto status = stub->GetUser(&context, request, &response);

        std::cout << std::format("[client without token] status.ok()= {}, ", status.ok());
        if (status.ok()) {
            std::cout << std::format("name= {}", response.name());
        } else {
            std::cout << std::format("code= {}, error= {}",
                static_cast<int>(status.error_code()),
                status.error_message());
        }
        std::cout << "\n";
    }

}

int main(const int argc, char *argv[]) {
    if (const std::string START_KIND{argc > 1 ? argv[1] : ""};
        START_KIND == "server") server_with_interceptor::start();
    else if (START_KIND == "client-i") client_with_interceptor::start();
    else if (START_KIND == "client") client_without_interceptor::start();
    else std::cout << "BAD KIND\n";

    return 0;
}
```

**Клиентский interceptor — добавление токена + логирование "снаружи"**

```cpp
class ClientAuthLoggingInterceptor : public grpc::experimental::Interceptor {
  void Intercept(InterceptorBatchMethods* methods) override {
    if (methods->QueryInterceptionHookPoint(PRE_SEND_INITIAL_METADATA)) {
      methods->GetSendInitialMetadata()->insert({"authorization", "Bearer secret-token-123"});
    }
    if (methods->QueryInterceptionHookPoint(POST_RECV_STATUS)) {
      // логируем время и статус
    }
    methods->Proceed();  // ключевой момент — без этого RPC зависнет навсегда
  }
};
```

В логе видно: `[client-interceptor] -> GetUser: добавлен auth-токен` — сам код вызова (`client_with_interceptor.cc`) вообще не упоминает токен, он появляется полностью на уровне инфраструктуры канала. Это и есть главная практическая ценность interceptor'ов: убрать повторяющийся boilerplate (auth, логирование, трейсинг) из каждого места вызова в одно центральное место.

**`QueryInterceptionHookPoint` — один `Intercept()` на всё**

Один и тот же метод `Intercept()` вызывается на **разных точках** жизненного цикла RPC — нужно спрашивать `QueryInterceptionHookPoint(...)`, чтобы понять, какая именно точка сейчас (отправка метаданных, отправка сообщения, получение статуса и т.д.), и реагировать только на нужные. `Proceed()` обязателен на **каждый** вызов `Intercept()` — иначе gRPC не продолжит обработку.

**Серверный interceptor — логирование + условная подмена статуса**

```cpp
if (QueryInterceptionHookPoint(POST_RECV_INITIAL_METADATA)) {
  // читаем token из GetRecvInitialMetadata()
}
if (QueryInterceptionHookPoint(PRE_SEND_STATUS)) {
  if (token != "Bearer secret-token-123") {
    methods->ModifySendStatus(Status(UNAUTHENTICATED, "invalid or missing token"));
  }
}
```

Лог сервера показывает оба прогона: с валидным токеном — `токен валиден, статус не трогаем`, с пустым — `подменяю статус на UNAUTHENTICATED`. Клиент без токена реально получил `код=16` (числовое значение `grpc::StatusCode::UNAUTHENTICATED`) вместо данных пользователя.

**Важная честная деталь, которую стоит знать**

В логе видно `[handler] GetUser(user_id=1)` **в обоих** случаях — включая запрос без токена. Это значит: подмена статуса на `PRE_SEND_STATUS` происходит **после** того, как обработчик уже выполнился (он успел вернуть `Status::OK`, но на самом отправлении interceptor переписал итоговый статус). Для чистой логики (например, счётчиков, изменения данных) это может быть неприемлемо — обработчик всё же "поработал". Для настоящего short-circuit до вызова бизнес-логики в реальных системах чаще проверяют токен прямо в начале самого обработчика через `context->client_metadata()`, а interceptor оставляют для сквозного логирования/трейсинга/метрик, где не важно, успел ли отработать handler.

**Регистрация**

```cpp
// клиент
auto channel = grpc::experimental::CreateCustomChannelWithInterceptors(
    address, creds, args, std::move(client_interceptor_creators));

// сервер
builder.experimental().SetInterceptorCreators(std::move(server_interceptor_creators));
```

Обрати внимание на `experimental` в названиях — в grpc 1.30 (версия из этой песочницы) interceptor API официально помечен экспериментальным (хотя стабилен и широко используется на практике); в более новых версиях gRPC он давно стабилизирован и живёт без этого namespace-префикса.
