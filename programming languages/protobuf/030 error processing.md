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
project(grpc_resilience_service CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# С vcpkg (см. vcpkg.json — dependencies: grpc, protobuf) оба пакета
# ставят свои CMake config-файлы, поэтому используем CONFIG-режим,
# а не module-режим (find_package(Protobuf) без CONFIG).
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

set(PROTO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/resilience_service.proto)
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_DIR})

set(GENERATED_SRCS
        ${GENERATED_DIR}/resilience_service.pb.cc
        ${GENERATED_DIR}/resilience_service.pb.h
        ${GENERATED_DIR}/resilience_service.grpc.pb.cc
        ${GENERATED_DIR}/resilience_service.grpc.pb.h
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
        COMMENT "Generation *.pb.* and *.grpc.pb.* from resilience_service.proto"
        VERBATIM
)

add_library(resilience_proto_grpc OBJECT ${GENERATED_SRCS})
target_include_directories(resilience_proto_grpc PUBLIC ${GENERATED_DIR})
target_link_libraries(resilience_proto_grpc PUBLIC protobuf::libprotobuf gRPC::grpc++)

add_executable(demo main.cpp)
target_link_libraries(demo PRIVATE resilience_proto_grpc)

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

### resilience_service.proto
```protobuf
syntax = "proto3";
package myapp;

message PingRequest {}
message PingResponse {
  string message = 1;
}

message SlowRequest {}
message SlowResponse {}

service ResilienceService {
  // Первые 2 попытки — искусственно возвращает UNAVAILABLE (для демо retry).
  rpc Ping(PingRequest) returns (PingResponse);

  // Специально "спит" дольше, чем клиентский deadline (для демо таймаутов).
  rpc SlowCall(SlowRequest) returns (SlowResponse);
}

```

### main.cpp
```cpp
#include <atomic>
#include <chrono>
#include <iostream>
#include <format>
#include <memory>
#include <thread>
#include <string>

#include <grpcpp/grpcpp.h>

#include "resilience_service.grpc.pb.h"

namespace {
    const std::string ADDRESS{"127.0.0.1:5000"};

    // ### SERVER ###
    std::atomic<int> g_ping_attempts{0};

    class ResilienceServiceImpl final: public myapp::ResilienceService::Service {
        grpc::Status Ping(grpc::ServerContext *context,
                    const myapp::PingRequest*,
                    myapp::PingResponse *response) override {
            const int attempt{++g_ping_attempts};
            std::cout << std::format("[server] ping, attempt #{}\n", attempt);

            if (attempt <= 2) {
                // Искусственный сбой первых двух попыток — код UNAVAILABLE считается
                // "временной проблемой", именно такие коды retry-политики обычно
                // помечают как retryable.
                return grpc::Status{grpc::StatusCode::UNAVAILABLE, "server temporarily overloaded"};
            }

            response->set_message(std::format("pong (succeeded on attempt {})", attempt));
            return grpc::Status::OK;
        }

        grpc::Status SlowCall(grpc::ServerContext *context,
                        const myapp::SlowRequest*,
                        myapp::SlowResponse*) override {
            std::cout << "[server] SlowCall starts 'work' (sleep 2 seconds)\n";
            for (int i{}; i < 20; ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                if (context->IsCancelled()) {
                    // Клиент уже отвалился по deadline — дальше работать бессмысленно.
                    std::cout << std::format("[server] discovered: client cancelled call (deadline), "
                                             "cancelling work on step {}\n", i);
                    return grpc::Status{grpc::StatusCode::CANCELLED, "cancelled by client deadline"};
                }
            }
            std::cout << "[server] SlowCall finished work\n";

            return grpc::Status::OK;
        }
    };

    void start_server() {
        ResilienceServiceImpl service;

        grpc::ServerBuilder builder;
        builder.AddListeningPort(ADDRESS, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);

        std::unique_ptr<grpc::Server> server = builder.BuildAndStart();
        std::cout << std::format("[server] listen to {}\n", ADDRESS);
        server->Wait();
    }

    // ### CLIENT ###

    // ===== 1. Deadline / timeout =====
    void demoDeadline() {
        std::cout << "\n=== Deadline: SlowCall with deadline=500ms (server will sleep 2s) ===\n";
        auto channel = grpc::CreateChannel(ADDRESS, grpc::InsecureChannelCredentials());
        auto stub = myapp::ResilienceService::NewStub(channel);

        grpc::ClientContext context;
        auto deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(500);
        context.set_deadline(deadline);

        myapp::SlowRequest request;
        myapp::SlowResponse response;
        auto start{std::chrono::steady_clock::now()};
        grpc::Status status{stub->SlowCall(&context, request, &response)};
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start).count();

        std::cout << std::format("[client] concluded in {} ms (and not in ~2000ms)\n", elapsed);
        std::cout << std::format("[client] status.ok()= {}, code= {}, {}, message= {}\n",
            status.ok(),
            static_cast<int>(status.error_code()),
            (status.error_code() == grpc::StatusCode::DEADLINE_EXCEEDED ? "DEADLINE_EXCEEDED" : "OTHER"),
            status.error_message());
    }

    // ===== 2. Retry policy через service config =====
    void demoRetry() {
        std::cout << "\n=== Retry: Ping with retry-policy (server breaks first two attempts) ===\n";
        const char* service_config_json = R"JSON(
        {
          "methodConfig": [{
            "name": [{"service": "myapp.ResilienceService", "method": "Ping"}],
            "retryPolicy": {
              "maxAttempts": 5,
              "initialBackoff": "0.1s",
              "maxBackoff": "1s",
              "backoffMultiplier": 2,
              "retryableStatusCodes": ["UNAVAILABLE"]
            }
          }]
        })JSON";

        grpc::ChannelArguments args;
        args.SetString(GRPC_ARG_SERVICE_CONFIG, service_config_json);
        args.SetInt(GRPC_ARG_ENABLE_RETRIES, 1);

        auto channel = grpc::CreateCustomChannel(
            ADDRESS, grpc::InsecureChannelCredentials(), args);
        auto stub = myapp::ResilienceService::NewStub(channel);

        grpc::ClientContext context;
        myapp::PingRequest request;
        myapp::PingResponse response;

        std::cout << "[client] will do one call stub->Ping(...)\n";
        grpc::Status status = stub->Ping(&context, request, &response);

        std::cout << "[client] status.ok()=" << status.ok() << "\n";
        if (status.ok()) {
          std::cout << "[client] answer: " << response.message()
                     << "  <- gRPC made the call again internally; our code didn't do that.\n";
        } else {
          std::cout << "[client] error: " << status.error_message() << "\n";
        }
    }

    void start_client() {
        demoDeadline();
        demoRetry();
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

**`grpc::Status` — базовая структура ошибки**

```cpp
Status(grpc::StatusCode::UNAVAILABLE, "server temporarily overloaded");
```

Три части: код (`StatusCode` enum), человекочитаемое сообщение, и опционально бинарные детали (`error_details()`). `Status::OK` — успех, всё остальное — ошибка; клиент всегда обязан проверять `status.ok()`, а не полагаться на содержимое ответа (это уже разбирали в теме про unary RPC).

**Коды ошибок — те, что реально встретились в демо**

- `DEADLINE_EXCEEDED` (код 4) — клиент не дождался ответа в заданный срок. Видно в выводе: `status.error_code()=4`, `message=Deadline Exceeded`.
- `CANCELLED` — сервер обнаружил, что клиент уже отвалился (через `context->IsCancelled()`), и сам прекратил работу вместо того, чтобы досчитать впустую (лог: `клиент отменил вызов ... прекращаю работу на шаге 4`).
- `UNAVAILABLE` (то, что возвращал `Ping` на первых двух попытках) — стандартный код для "временная проблема, можно повторить". Именно поэтому он попал в список `retryableStatusCodes`.
- Из более ранних демо: `NOT_FOUND` (несуществующий пользователь).

Полный список кодов гораздо шире (`INVALID_ARGUMENT`, `ALREADY_EXISTS`, `PERMISSION_DENIED`, `RESOURCE_EXHAUSTED`, `FAILED_PRECONDITION`, `UNAUTHENTICATED`, `INTERNAL`, `UNIMPLEMENTED` и др.) — общий принцип: коды описывают **категорию** проблемы (похоже на HTTP-статусы, но заточено под RPC-семантику), а не конкретную бизнес-ошибку — для бизнес-деталей используют `error_details()` или прикладной код внутри самого response-сообщения.

**Deadlines/timeouts**

```cpp
auto deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(500);
context.set_deadline(deadline);
```

В выводе видно: вызов, который на сервере занял бы ~2000 мс, реально завершился за **500 мс** — клиент не ждал полного времени работы сервера, `ClientContext` сам оборвал ожидание по достижении дедлайна. При этом сервер узнал об этом (`context->IsCancelled()` внутри цикла `sleep_for`) и прекратил бесполезную работу, а не продолжал считать вхолостую ещё 1.5 секунды — это важная деталь: без явной проверки `IsCancelled()` сервер продолжил бы работать, просто клиент бы этого уже не дождался.

Дедлайн — это абсолютная точка во времени (`now() + duration`), а не просто "таймаут в секундах"; при передаче вызова дальше по цепочке сервисов (A → B → C) дедлайн естественно "утекает" вместе с оставшимся временем, если его правильно прокидывать через контекст.

**Retry-политика через service config**

```json
{
  "methodConfig": [{
    "name": [{"service": "myapp.ResilienceService", "method": "Ping"}],
    "retryPolicy": {
      "maxAttempts": 5,
      "initialBackoff": "0.1s",
      "maxBackoff": "1s",
      "backoffMultiplier": 2,
      "retryableStatusCodes": ["UNAVAILABLE"]
    }
  }]
}
```

```cpp
ChannelArguments args;
args.SetString(GRPC_ARG_SERVICE_CONFIG, service_config_json);
args.SetInt(GRPC_ARG_ENABLE_RETRIES, 1);
auto channel = grpc::CreateCustomChannel(address, creds, args);
```

Код клиента сделал **ровно один** вызов `stub->Ping(...)` — никакого ручного цикла `for (retry...)`. Лог сервера показывает 3 реальные попытки (`#1`, `#2`, `#3`), первые две вернули `UNAVAILABLE`, а клиентский код увидел только финальный успешный результат: `status.ok()=1`, `pong (succeeded on attempt 3)`. Это и есть суть retry-политики — она полностью прозрачна для вызывающего кода, встроена в сам gRPC-канал: экспоненциальный backoff (`initialBackoff` × `backoffMultiplier` на каждой попытке, не превышая `maxBackoff`) между попытками, ограничение `maxAttempts`, и повтор только для кодов из `retryableStatusCodes` (не любая ошибка ретраится — например, `INVALID_ARGUMENT` повторять бессмысленно, ошибка не исчезнет).

**Практическое правило**

Deadline — всегда ставь явно на клиенте (дефолт — бесконечность, что почти всегда плохая идея в проде). Retry-политику — конфигурируй через service config на уровне канала, а не пиши руками: библиотека уже учитывает backoff и защиту от "штурма повторов" (retry storm), которую легко испортить самодельным циклом.
