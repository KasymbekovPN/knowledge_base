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
project(grpc_chat_service CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# С vcpkg (см. vcpkg.json — dependencies: grpc, protobuf) оба пакета
# ставят свои CMake config-файлы, поэтому используем CONFIG-режим,
# а не module-режим (find_package(Protobuf) без CONFIG).
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

set(PROTO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/chat_service.proto)
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_DIR})

set(GENERATED_SRCS
        ${GENERATED_DIR}/chat_service.pb.cc
        ${GENERATED_DIR}/chat_service.pb.h
        ${GENERATED_DIR}/chat_service.grpc.pb.cc
        ${GENERATED_DIR}/chat_service.grpc.pb.h
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
        COMMENT "Generation *.pb.* and *.grpc.pb.* from chat_service.proto"
        VERBATIM
)

add_library(chat_proto_grpc OBJECT ${GENERATED_SRCS})
target_include_directories(chat_proto_grpc PUBLIC ${GENERATED_DIR})
target_link_libraries(chat_proto_grpc PUBLIC protobuf::libprotobuf gRPC::grpc++)

add_executable(demo main.cpp)
target_link_libraries(demo PRIVATE chat_proto_grpc)

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

### chat_service.proto
```protobuf
syntax = "proto3";
package myapp;

message ChatMessage {
  string from = 1;
  string text = 2;
}

service ChatService {
  //  // Оба потока (клиент->сервер и сервер->клиент) полностью независимы:
  //  // ни одна сторона не обязана отвечать "один к одному" на сообщение другой.
  rpc Chat(stream ChatMessage) returns (stream ChatMessage);
}

```

### main.cpp
```cpp
#include <atomic>
#include <chrono>
#include <iostream>
#include <format>
#include <memory>
#include <mutex>
#include <thread>

#include <grpcpp/grpcpp.h>

#include "chat_service.grpc.pb.h"

namespace {

    std::string now_ms() {
        const auto now{std::chrono::system_clock::now().time_since_epoch()};
        const auto ms{std::chrono::duration_cast<std::chrono::milliseconds>(now).count()};

        return std::to_string(ms % 100'000);
    }

    // ### SERVER ###
    class ChatServiceImpl final: public myapp::ChatService::Service {
        grpc::Status Chat(grpc::ServerContext* context,
                    grpc::ServerReaderWriter<myapp::ChatMessage, myapp::ChatMessage>* stream) override {

            std::atomic<bool> client_done{false};
            // Write() из разных потоков нужно синхронизировать
            std::mutex write_mutex;

            // Отдельный поток: сервер САМ, независимо от клиента, шлёт heartbeat —
            // это и есть суть bidi: сервер не "отвечает" на сообщения клиента,
            // а пишет в поток по собственному расписанию.
            std::thread heartbeat_thread{[&]() {
                int n{0};
                while (!client_done.load()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(350));
                    if (client_done.load()) break;

                    myapp::ChatMessage hb;
                    hb.set_from("server");
                    hb.set_text(std::format("heart beat #{}", ++n));
                    {
                        std::lock_guard<std::mutex> lock{write_mutex};
                        stream->Write(hb);
                    }
                    std::cout << std::format("[t= {}] [server, thread heartbeat] -> '{}'\n", now_ms(), hb.text());
                }
            }};

            // Основной поток RPC-обработчика: читает то, что шлёт клиент,
            // и эхом отвечает — тоже пишет в тот же stream, но из другого потока.
            myapp::ChatMessage msg;
            while (stream->Read(&msg)) {
                std::cout << std::format("[{}] [server read-thread] <- {}: '{}'\n",
                    now_ms(),
                    msg.from(),
                    msg.text());

                myapp::ChatMessage reply;
                reply.set_from("server");
                reply.set_text(std::format("echo: '{}'", msg.text()));
                {
                    std::lock_guard<std::mutex> lock{write_mutex};
                    stream->Write(reply);
                }

                std::cout << std::format("[t= {}] [read thread] -> '{}'\n", now_ms(), reply.text());
            }

            client_done.store(true);
            heartbeat_thread.join();
            std::cout << std::format("[t= {}] [server] client finished stream\n", now_ms());

            return grpc::Status::OK;
        }
    };

    void start_server() {
        std::string address{"127.0.0.1:5001"};
        ChatServiceImpl service;

        grpc::ServerBuilder builder;
        builder.AddListeningPort(address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);

        std::unique_ptr<grpc::Server> server = builder.BuildAndStart();
        std::cout << std::format("[server] listen to {}\n", address);
        server->Wait();
    }

    // ### CLIENT ###

    void start_client() {
        std::string address{"127.0.0.1:5001"};
        auto channel = grpc::CreateChannel(address, grpc::InsecureChannelCredentials());
        std::unique_ptr<myapp::ChatService::Stub> stub = myapp::ChatService::NewStub(channel);

        grpc::ClientContext context;
        std::shared_ptr<grpc::ClientReaderWriter<myapp::ChatMessage, myapp::ChatMessage>> stream{
            stub->Chat(&context)
        };

        // Отдельный поток ТОЛЬКО читает — независимо от того, когда и что
        // пишет основной поток. Именно это делает bidi bidi, а не ping-pong.
        std::thread reader_thread{[stream]() {
            myapp::ChatMessage msg;
            while (stream->Read(&msg)) {
                std::cout << std::format("[t = {}] [client, read-thread] <- {}: '{}'\n",
                    now_ms(), msg.from(), msg.text());
            }
            std::cout << std::format("[t = {}] [client, read-thread] server closed stream\n", now_ms());
        }};

        // Основной поток пишет с произвольным интервалом, не оглядываясь
        // на то, что и когда приходит в ответ.
        for (const std::string& text: {"hello", "what do you do?", "Bye"}) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            myapp::ChatMessage msg;
            msg.set_from("client");
            msg.set_from(text);
            stream->Write(msg);
            std::cout << std::format("[t= {}] [client, main thread] -> '{}'\n", now_ms(), text);
        }

        stream->WritesDone();
        reader_thread.join();

        std::cout << std::format("[client] Chat completed, ok = {}\n", stream->Finish().ok());
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

**Ключевая идея, которую доказывает этот прогон**

В прошлом демо (`SyncOrders`) я показывал "синхронный ping-pong" — write-then-immediately-read, что по сути имитирует unary-вызов внутри одного стрима. Здесь — по-настоящему независимые направления: сервер завёл отдельный поток `heartbeat_thread`, который пишет в `stream` каждые 350 мс **вне зависимости** от того, что и когда пишет клиент. Основной поток обработчика сервера тем временем читает сообщения клиента и отвечает эхом. Оба потока пишут в один и тот же `ServerReaderWriter*`, синхронизируясь через `std::mutex` — это прямо в коде:

```cpp
std::mutex write_mutex;
// поток 1: heartbeat
{ std::lock_guard<std::mutex> lock(write_mutex); stream->Write(hb); }
// поток 2 (основной): эхо на чтение клиента
{ std::lock_guard<std::mutex> lock(write_mutex); stream->Write(reply); }
```

**Почему нужен mutex, а не просто два потока пишущих одновременно**

gRPC C++ sync API гарантирует потокобезопасность только для **одного** одновременного вызова `Write()` и **одного** одновременного вызова `Read()` — то есть один поток может писать, другой одновременно читать, но два потока не могут писать одновременно без внешней синхронизации (иначе байты одного `Write` могут перемешаться с байтами другого). У меня в демо ровно такая раскладка: один читающий поток (не требует mutex — единственный), два потенциально пишущих потока на сервере (требуют mutex между собой).

**Клиентская сторона — то же самое, зеркально**

```cpp
std::thread reader_thread([stream]() {
  while (stream->Read(&msg)) { ... }
});
// основной поток пишет независимо от reader_thread
stream->Write(msg);
```

Клиент тоже развёл чтение и запись по разным потокам — именно поэтому в логе видно, что heartbeat #1 пришёл клиенту (`t=92289`) даже раньше, чем клиент вообще успел что-то отправить (первое `-> привет` только на `t=92438`). Если бы это была последовательная логика "write → read → write → read", такое было бы физически невозможно.

**Завершение стрима**

```cpp
stream->WritesDone();  // клиент сигнализирует "больше писать не буду"
reader_thread.join();   // но читающий поток продолжает получать данные,
                         // пока сервер сам не закроет свою половину
Status status = stream->Finish();
```

На сервере `stream->Read()` возвращает `false`, когда клиент вызвал `WritesDone()` — сервер это увидел (`[сервер] клиент завершил стрим` на `t=93691`), после чего сам завершил `heartbeat_thread` и вернул `Status::OK`. Обрати внимание: клиент перестал писать раньше (`WritesDone` сразу после трёх сообщений), но продолжал **читать** ещё какое-то время (heartbeat #4 пришёл уже после последнего `Write` клиента) — направления действительно независимы до самого конца.
