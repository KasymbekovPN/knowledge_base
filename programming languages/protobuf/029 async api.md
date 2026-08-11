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

service UserService {
  rpc GetUser(UserRequest) returns (UserResponse);
}

```

### main.cpp
```cpp
#include <iostream>
#include <format>
#include <memory>
#include <string>
#include <thread>

#include <grpcpp/grpcpp.h>

#include "user_service.grpc.pb.h"

namespace {
    // Состояние ОДНОГО асинхронного вызова. При масштабировании до многих
    // параллельных запросов на каждый заводят свой такой объект — "тег",
    // по которому grpc::CompletionQueue сообщает, что именно завершилось.
    struct AsyncCall {
        myapp::UserResponse response;
        grpc::ClientContext context;
        grpc::Status status;
        std::unique_ptr<grpc::ClientAsyncResponseReader<myapp::UserResponse>> response_reader;
    };
}

int main() {
    std::string address{"127.0.0.1:5001"};
    auto channel = grpc::CreateChannel(address, grpc::InsecureChannelCredentials());
    std::unique_ptr<myapp::UserService::Stub> stub = myapp::UserService::NewStub(channel);

    // общая очередь завершённых операций для этого клиента
    grpc::CompletionQueue cq;

    // --- Запускаем ДВА асинхронных вызова "одновременно" (не блокируя поток) ---
    const auto call1 = new AsyncCall();
    const auto call2 = new AsyncCall();

    {
        myapp::UserRequest req;
        req.set_user_id(1);
        call1->response_reader = stub->PrepareAsyncGetUser(&call1->context, req, &cq);
        call1->response_reader->StartCall();
        // tag = указатель на call1 — так grpc::CompletionQueue скажет нам,
        // какой именно вызов завершился
        call1->response_reader->Finish(&call1->response, &call1->status, static_cast<void *>(call1));
        std::cout << "[client] request #1 (user_id=1) sent, do not wait answer\n";
    }

    {
        myapp::UserRequest req;
        req.set_user_id(999);
        call2->response_reader = stub->PrepareAsyncGetUser(&call2->context, req, &cq);
        call2->response_reader->StartCall();
        call2->response_reader->Finish(&call2->response, &call2->status, static_cast<void *>(call2));
        std::cout << "[client] request #2 (user_id=999) sent, do not wait answer\n";
    }

    std::cout << "[client] both calling is async, in-process, thread is free\n";

    // --- Забираем результаты по мере готовности через cq.Next() ---
    // Порядок завершения НЕ гарантирован — сеть/сервер может ответить в любом порядке.
    for (int i{}; i < 2; ++i) {
        void* got_tag;
        bool ok{false};
        // блокирует ТОЛЬКО пока нет ни одного результата
        cq.Next(&got_tag, &ok);

        AsyncCall* call{static_cast<AsyncCall*>(got_tag)};
        if (ok && call->status.ok()) {
            std::cout << "[client] gotten answer for tag=" << got_tag << ": "
                << call->response.name() << "<" << call->response.email() << ">\n";
        } else {
            std::cout << "[client] gotten error for tag=" << got_tag << ": "
                << call->status.error_message() << "\n";
        }
        delete call;
    }

    cq.Shutdown();

    return 0;
}


```


**Зачем `CompletionQueue`**

В синхронном API каждый вызов (`stub->GetOrder(...)`) блокирует поток до получения ответа — на N параллельных запросов нужно N потоков. `CompletionQueue` — это очередь, в которую gRPC кладёт уведомления о завершившихся операциях (запрос отправлен, ответ получен, поток закрыт и т.д.), а один поток может обслуживать сколько угодно параллельных RPC, просто вычитывая эту очередь. В демо видно: оба запроса отправлены практически мгновенно (`[клиент] оба вызова асинхронно "в полёте", поток свободен`), а результаты пришли позже и не по порядку отправки.

**Механика клиентского вызова**

```cpp
auto reader = stub->PrepareAsyncGetUser(&context, req, &cq);  // подготовили, НЕ отправили
reader->StartCall();                                            // отправили запрос
reader->Finish(&response, &status, (void*)call);                // "когда будет готов ответ — положи в cq с этим тегом"
```

`PrepareAsync*` только готовит вызов, реальная отправка происходит на `StartCall()`. `Finish()` не блокирует — просто регистрирует, куда и с каким **тегом** (`void*`) положить результат, когда он появится.

**Тег — как отличить, какой из многих вызовов завершился**

```cpp
void* got_tag;
bool ok;
cq.Next(&got_tag, &ok);  // блокирует, ПОКА нет ни одного готового результата
AsyncCall* call = static_cast<AsyncCall*>(got_tag);
```

Тег — это просто указатель, который сам код передал при регистрации операции (`(void*)call1`, `(void*)call2`), и который gRPC возвращает обратно нетронутым. По нему приложение понимает, к какому именно `AsyncCall`/состоянию относится это уведомление — в реальных сервисах с сотнями одновременных запросов это единственный способ сопоставить событие с контекстом.

**`ok` — важный, но не всегда очевидный флаг**

Для клиентского unary-вызова `ok=true` означает, что RPC действительно завершился (успешно или с ошибкой — это отдельно проверяется через `call->status`); `ok=false` означало бы проблему на уровне самого канала/очереди (реже встречается в клиентском unary-сценарии, куда чаще актуально для потоковых операций и серверной стороны).

**Почему асинхронный сервер сложнее (и почему не стал его разворачивать здесь)**

На сервере с `CompletionQueue` вместо простого переопределения `Service::GetUser()` пишут ручной конечный автомат: тег регистрируется на этапе "готов принять новый запрос" (`RequestGetUser`), затем на этапе "запрос принят, обрабатываю", затем "ответ отправлен" — и каждый переход состояния явно кодируется программистом (в отличие от синхронного API, где всё это скрыто внутри библиотеки). Это заметно более многословный и подверженный ошибкам код (нужно вручную управлять временем жизни каждого объекта-состояния, как я сделал вручную через `new AsyncCall()`/`delete call` в демо) — именно поэтому на практике для async-сервера чаще берут готовые обёртки повыше уровнем (например, C++ callback-based API `grpc::CallbackService`, появившийся в более новых версиях gRPC, чем 1.30 в этой песочнице) вместо голого `CompletionQueue`.

**Практическое правило**

`CompletionQueue` на клиенте — оправдан, когда нужно держать много параллельных RPC без пропорционального роста числа потоков (клиент высоконагруженного сервиса, который сам вызывает другие сервисы). На сервере — оправдан для экстремально высокой пропускной способности, где накладные расходы на поток-на-соединение синхронного API становятся узким местом; для большинства сервисов синхронный APIдостаточен и значительно проще в сопровождении.
