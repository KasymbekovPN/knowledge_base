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
project(grpc_order_service CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# С vcpkg (см. vcpkg.json — dependencies: grpc, protobuf) оба пакета
# ставят свои CMake config-файлы, поэтому используем CONFIG-режим,
# а не module-режим (find_package(Protobuf) без CONFIG).
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

set(PROTO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/order_service.proto)
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_DIR})

set(GENERATED_SRCS
        ${GENERATED_DIR}/order_service.pb.cc
        ${GENERATED_DIR}/order_service.pb.h
        ${GENERATED_DIR}/order_service.grpc.pb.cc
        ${GENERATED_DIR}/order_service.grpc.pb.h
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

add_library(order_proto_grpc OBJECT ${GENERATED_SRCS})
target_include_directories(order_proto_grpc PUBLIC ${GENERATED_DIR})
target_link_libraries(order_proto_grpc PUBLIC protobuf::libprotobuf gRPC::grpc++)

add_executable(demo_grpc_sync main.cpp)
target_link_libraries(demo_grpc_sync PRIVATE order_proto_grpc)

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

### order_service.proto
```Protobuf
syntax = "proto3";
package myapp;

// Сообщения для примера — минимальные, специально под демонстрацию service
message OrderRequest {
  int64 order_id = 1;
}

message OrderResponse {
  int64  order_id = 1;
  string status = 2;
}

message OrderUpdate {
  int64 order_id = 1;
  string field = 2;
  string value = 3;
}

message Ack {
  bool ok = 1;
}

service OrderService {
  // 1. Unary — один запрос, один ответ. Самый частый вид RPC,
  //    ведёт себя как обычный вызов функции по сети.
  rpc GetOrder(OrderRequest) returns (OrderResponse);

  // 2. Server streaming — один запрос, СЕРВЕР шлёт поток ответов.
  //    Клиент запросил один раз, сервер отдаёт данные порциями
  //    (например, историю статусов заказа).
  rpc WatchOrderStatus(OrderRequest) returns (stream OrderResponse);

  // 3. Client streaming — КЛИЕНТ шлёт поток запросов, сервер отвечает один раз
  //    в конце (например, серия обновлений полей заказа, применяются пачкой).
  rpc BatchUpdateOrder(stream OrderUpdate) returns (Ack);

  // 4. Bidi (двунаправленный) streaming — оба потока независимы,
  //    клиент и сервер шлют сообщения в любом порядке по одному соединению.
  rpc SyncOrders(stream OrderUpdate) returns (stream OrderResponse);
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
#include "order_service.grpc.pb.h"

namespace {
    // ===================== СЕРВЕР =====================
    // Реализуем Service — базовый класс уже даёт дефолтную реализацию
    // (UNIMPLEMENTED) для всех методов, переопределяем только нужные.
    class OrderServiceImpl: public myapp::OrderService::Service {
        grpc::Status GetOrder(grpc::ServerContext *context,
                              const myapp::OrderRequest *request,
                              myapp::OrderResponse *response) override {

            response->set_order_id(request->order_id());
            response->set_status("PAID");

            return grpc::Status::OK;
        }

        grpc::Status WatchOrderStatus(grpc::ServerContext *context,
                                      const myapp::OrderRequest *request,
                                      grpc::ServerWriter<myapp::OrderResponse> *writer) override {
            for (const char* statuses[] = {"PENDING", "PAID", "SHIPPED", "DELIVERED"};
                const char* s: statuses) {
                myapp::OrderResponse resp;
                resp.set_order_id(request->order_id());
                resp.set_status(s);
                // сервер сам решает, сколько раз писать в поток
                writer->Write(resp);
            }

            return grpc::Status::OK;
        }

        grpc::Status BatchUpdateOrder(grpc::ServerContext *context,
                                      grpc::ServerReader<myapp::OrderUpdate> *reader,
                                      myapp::Ack *response) override {
            myapp::OrderUpdate update;
            int count{};
            // читаем, пока клиент не завершит поток
            while (reader->Read(&update)) {
                ++count;
                std::cout << std::format("  [server] updating received #{}: = {}\n",
                    update.field(),
                    update.value());
            }
            response->set_ok(count > 0);

            return grpc::Status::OK;
        }

        grpc::Status SyncOrders(grpc::ServerContext *context,
                                grpc::ServerReaderWriter<myapp::OrderResponse, myapp::OrderUpdate> *stream) override {
            myapp::OrderUpdate update;
            // читаем один запрос клиента...
            while (stream->Read(&update)) {
                myapp::OrderResponse resp;
                resp.set_order_id(update.order_id());
                resp.set_status(std::format("ACK:{}", update.field()));
                stream->Write(resp); // ...и сразу отвечаем — в рамках одного соединения
            }

            return grpc::Status::OK;
        }
    };

    void runServer(std::unique_ptr<grpc::Server>* out_server, const std::string& address) {
        OrderServiceImpl service;
        grpc::ServerBuilder builder;
        builder.AddListeningPort(address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
        *out_server = builder.BuildAndStart();
        std::cout << std::format("  [server] listening to {}\n", address);
        (*out_server)->Wait();  // блокирует поток до Shutdown()
    }

    // ===================== КЛИЕНТ =====================
    void runClient(const std::string& address) {
        auto channel{grpc::CreateChannel(address, grpc::InsecureChannelCredentials())};
        std::unique_ptr<myapp::OrderService::Stub> stub{myapp::OrderService::NewStub(channel)};

        // --- 1. Unary ---
        {
            grpc::ClientContext context;
            myapp::OrderRequest req;
            req.set_order_id(42);
            myapp::OrderResponse resp;
            const grpc::Status status{stub->GetOrder(&context, req, &resp)};
            std::cout << std::format("\n  [client] Unary GetOrder: ok= {}, status= {}\n",
                status.ok(),
                resp.status());
        }

        // --- 2. Server streaming ---
        {
            grpc::ClientContext context;
            myapp::OrderRequest req;
            req.set_order_id(42);
            std::unique_ptr<grpc::ClientReader<myapp::OrderResponse>> reader{
                stub->WatchOrderStatus(&context, req)
            };
            std::cout << "[client] Server streaming WatchOrderStatus:\n";
            myapp::OrderResponse resp;
            while (reader->Read(&resp)) {
                std::cout << std::format(" <- {}\n", resp.status());
            }
            std::cout << std::format("  Finish ok= {}\n", reader->Finish().ok());
        }

        // --- 3. Client streaming ---
        {
            grpc::ClientContext context;
            myapp::Ack ack;
            std::unique_ptr<grpc::ClientWriter<myapp::OrderUpdate>> writer{
                stub->BatchUpdateOrder(&context, &ack)
            };
            std::cout << "[client] Client streaming BatchUpdateOrder:\n";
            for (const std::string& field: {"priority", "note"}) {
                myapp::OrderUpdate u;
                u.set_order_id(42);
                u.set_field(field);
                u.set_value("x");
                writer->Write(u);
                std::cout << std::format("  -> sent: {}\n", field);
            }

            writer->WritesDone();
            //     Status status = writer->Finish();
            std::cout << std::format("  Finish ok= {}, ack.ok()= {}\n",
                writer->Finish().ok(),
                ack.ok());
        }

        // --- 4. Bidi streaming ---
        {
            grpc::ClientContext context;
            std::unique_ptr<grpc::ClientReaderWriter<myapp::OrderUpdate, myapp::OrderResponse>> stream{
                stub->SyncOrders(&context)
            };
            std::cout << "[client] Bidi streaming SyncOrders:\n";
            for (const std::string& field: {"color", "size"}) {
                myapp::OrderUpdate u;
                u.set_order_id(42);
                u.set_field(field);
                stream->Write(u);
                myapp::OrderResponse resp;
                // синхронный ping-pong: пишем и сразу читаем ответ
                stream->Read(&resp);
                std::cout << std::format(" <-> {} => {}\n", field, resp.status());
            }

            stream->WritesDone();
            std::cout << std::format("  Finish ok= {}\n", stream->Finish().ok());
        }
    }

}

int main() {
    const std::string address{"127.0.0.1:50061"};
    std::unique_ptr<grpc::Server> server;

    std::thread server_thread{[&]() { runServer(&server, address); }};
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    runClient(address);

    server->Shutdown();
    server_thread.join();
    std::cout << "\n[server] stopped\n";

    return 0;
}

```
