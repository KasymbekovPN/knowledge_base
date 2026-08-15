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
    }
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

set(PROTO_FILE
        ${CMAKE_CURRENT_SOURCE_DIR}/user_service.proto
        ${CMAKE_CURRENT_SOURCE_DIR}/reflection_service.proto
)
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_DIR})

set(GENERATED_SRCS
        ${GENERATED_DIR}/user_service.pb.cc
        ${GENERATED_DIR}/user_service.pb.h
        ${GENERATED_DIR}/user_service.grpc.pb.cc
        ${GENERATED_DIR}/user_service.grpc.pb.h
        ${GENERATED_DIR}/reflection_service.pb.cc
        ${GENERATED_DIR}/reflection_service.pb.h
        ${GENERATED_DIR}/reflection_service.grpc.pb.cc
        ${GENERATED_DIR}/reflection_service.grpc.pb.h
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
target_link_libraries(user_proto_grpc PUBLIC protobuf::libprotobuf gRPC::grpc++ gRPC::grpc++_reflection)

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

### reflection_service.proto
```protobuf
// Copyright 2016 gRPC authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Service exported by server reflection

syntax = "proto3";

package grpc.reflection.v1alpha;

service ServerReflection {
  // The reflection service is structured as a bidirectional stream, ensuring
  // all related requests go to a single server.
  rpc ServerReflectionInfo(stream ServerReflectionRequest)
      returns (stream ServerReflectionResponse);
}

// The message sent by the client when calling ServerReflectionInfo method.
message ServerReflectionRequest {
  string host = 1;
  // To use reflection service, the client should set one of the following
  // fields in message_request. The server distinguishes requests by their
  // defined field and then handles them using corresponding methods.
  oneof message_request {
    // Find a proto file by the file name.
    string file_by_filename = 3;

    // Find the proto file that declares the given fully-qualified symbol name.
    // This field should be a fully-qualified symbol name
    // (e.g. <package>.<service>[.<method>] or <package>.<type>).
    string file_containing_symbol = 4;

    // Find the proto file which defines an extension extending the given
    // message type with the given field number.
    ExtensionRequest file_containing_extension = 5;

    // Finds the tag numbers used by all known extensions of the given message
    // type, and appends them to ExtensionNumberResponse in an undefined order.
    // Its corresponding method is best-effort: it's not guaranteed that the
    // reflection service will implement this method, and it's not guaranteed
    // that this method will provide all extensions. Returns
    // StatusCode::UNIMPLEMENTED if it's not implemented.
    // This field should be a fully-qualified type name. The format is
    // <package>.<type>
    string all_extension_numbers_of_type = 6;

    // List the full names of registered services. The content will not be
    // checked.
    string list_services = 7;
  }
}

// The type name and extension number sent by the client when requesting
// file_containing_extension.
message ExtensionRequest {
  // Fully-qualified type name. The format should be <package>.<type>
  string containing_type = 1;
  int32 extension_number = 2;
}

// The message sent by the server to answer ServerReflectionInfo method.
message ServerReflectionResponse {
  string valid_host = 1;
  ServerReflectionRequest original_request = 2;
  // The server set one of the following fields accroding to the message_request
  // in the request.
  oneof message_response {
    // This message is used to answer file_by_filename, file_containing_symbol,
    // file_containing_extension requests with transitive dependencies. As
    // the repeated label is not allowed in oneof fields, we use a
    // FileDescriptorResponse message to encapsulate the repeated fields.
    // The reflection service is allowed to avoid sending FileDescriptorProtos
    // that were previously sent in response to earlier requests in the stream.
    FileDescriptorResponse file_descriptor_response = 4;

    // This message is used to answer all_extension_numbers_of_type requst.
    ExtensionNumberResponse all_extension_numbers_response = 5;

    // This message is used to answer list_services request.
    ListServiceResponse list_services_response = 6;

    // This message is used when an error occurs.
    ErrorResponse error_response = 7;
  }
}

// Serialized FileDescriptorProto messages sent by the server answering
// a file_by_filename, file_containing_symbol, or file_containing_extension
// request.
message FileDescriptorResponse {
  // Serialized FileDescriptorProto messages. We avoid taking a dependency on
  // descriptor.proto, which uses proto2 only features, by making them opaque
  // bytes instead.
  repeated bytes file_descriptor_proto = 1;
}

// A list of extension numbers sent by the server answering
// all_extension_numbers_of_type request.
message ExtensionNumberResponse {
  // Full name of the base type, including the package name. The format
  // is <package>.<type>
  string base_type_name = 1;
  repeated int32 extension_number = 2;
}

// A list of ServiceResponse sent by the server answering list_services request.
message ListServiceResponse {
  // The information of each service may be expanded in the future, so we use
  // ServiceResponse message to encapsulate it.
  repeated ServiceResponse service = 1;
}

// The information of a single service used by ListServiceResponse to answer
// list_services request.
message ServiceResponse {
  // Full name of a registered service, including its package name. The format
  // is <package>.<service>
  string name = 1;
}

// The error code and error message sent by the server when an error occurs.
message ErrorResponse {
  // This field uses the error codes defined in grpc::StatusCode.
  int32 error_code = 1;
  string error_message = 2;
}
```
### main.cpp
```cpp
#include <iostream>
#include <format>
#include <reflection_service.grpc.pb.h>
#include <string>

#include <grpcpp/grpcpp.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <google/protobuf/descriptor.pb.h>

#include "user_service.grpc.pb.h"
#include "reflection_service.pb.h"

namespace {
    const std::string ADDRESS{"127.0.0.1:5000"};
}

namespace server {
    namespace {
        class UserServiceImpl final: public myapp::UserService::Service {
        public:
            grpc::Status GetUser(grpc::ServerContext *context, const myapp::UserRequest *request,
                myapp::UserResponse *response) override {

                response->set_user_id(request->user_id());
                response->set_name("Alice");
                response->set_email("alice@example.com");

                return grpc::Status::OK;
            }

            grpc::Status ListUsers(grpc::ServerContext *context, const myapp::ListUsersRequest *request,
                grpc::ServerWriter<myapp::UserResponse> *writer) override {

                myapp::UserResponse u;
                u.set_user_id(1);
                u.set_name("Alice");
                writer->Write(u);

                return grpc::Status::OK;
            }
        };

        void start() {
            UserServiceImpl service;

            // Единственная строка, которая включает reflection service — без неё
            // сервер вообще не знает, что кто-то может спросить его схему в рантайме.
            grpc::reflection::InitProtoReflectionServerBuilderPlugin();

            grpc::ServerBuilder builder;
            builder.AddListeningPort(ADDRESS, grpc::InsecureServerCredentials());
            builder.RegisterService(&service);

            std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
            std::cout << std::format("[server] listen to {} (reflection ON)\n", ADDRESS);
            server->Wait();
        }
    }

}

namespace client {
    // Работает не через user_service.grpc.pb.h не знает о существовании UserService на этапе компиляции.
    // Всё, что он узнаёт о сервисе (имя, методы, streaming или нет), приходит по сети
    // через reflection.proto — именно так работает grpcurl "без клиента".

    namespace {
        void start() {
            const auto channel{grpc::CreateChannel(ADDRESS, grpc::InsecureChannelCredentials())};
            std::unique_ptr<grpc::reflection::v1alpha::ServerReflection::Stub> stub{
                grpc::reflection::v1alpha::ServerReflection::NewStub(channel)
            };

            grpc::ClientContext context;
            std::shared_ptr<grpc::ClientReaderWriter<
                grpc::reflection::v1alpha::ServerReflectionRequest,
                grpc::reflection::v1alpha::ServerReflectionResponse>> stream{
                stub->ServerReflectionInfo(&context)
            };

            // ===== 1. list_services — аналог `grpcurl -plaintext list` =====
            {
                grpc::reflection::v1alpha::ServerReflectionRequest request;
                request.set_list_services("");
                stream->Write(request);

                grpc::reflection::v1alpha::ServerReflectionResponse response;
                stream->Read(&response);

                std::cout << "=== Servers list (gotten by net, without .proto on client) ===\n";
                for (const auto& svc: response.list_services_response().service()) {
                    std::cout << std::format(" {}\n", svc.name());
                }
            }

            // ===== 2. file_containing_symbol — аналог `grpcurl -plaintext describe` =====
            {
                grpc::reflection::v1alpha::ServerReflectionRequest request;
                request.set_file_containing_symbol("myapp.UserService");
                stream->Write(request);

                grpc::reflection::v1alpha::ServerReflectionResponse response;
                stream->Read(&response);

                std::cout << "\n=== Scheme myapp.UserService (reconstructed from wire) ===\n";
                for (const auto& bytes: response.file_descriptor_response().file_descriptor_proto()) {
                    google::protobuf::FileDescriptorProto file_proto;
                    file_proto.ParseFromString(bytes);

                    for (const auto& svc: file_proto.service()) {
                        std::cout << std::format("service {} {{\n", svc.name());
                        for (const auto& method: svc.method()) {
                            std::cout << std::format("  rpc {}({}{}) returns ({}{});\n",
                                method.name(),
                                (method.client_streaming() ? "stream" : ""),
                                method.input_type(),
                                (method.server_streaming() ? "stream" : ""),
                                method.output_type());
                        }
                    }
                    std::cout << "}\n";
                }
            }

            stream->WritesDone();
            std::cout << std::format("\n[client] reflection-session completed, ok= {}\n",
                stream->Finish().ok());
        }
    }

}

int main(const int argc, char *argv[]) {
    if (const std::string START_KIND{argc > 1 ? argv[1] : ""};
        START_KIND == "server") server::start();
    else if (START_KIND == "client") client::start();
    else std::cout << "BAD KIND\n";

    return 0;
}

```

**Как включить reflection на сервере**

```cpp
#include <grpcpp/ext/proto_server_reflection_plugin.h>
...
grpc::reflection::InitProtoReflectionServerBuilderPlugin();
ServerBuilder builder;
builder.RegisterService(&service);
```

Одна строчка регистрирует специальный служебный сервис `grpc.reflection.v1alpha.ServerReflection` рядом с твоими обычными сервисами. Он экспортирует метаданные обо всех **других** зарегистрированных сервисах — то, что раньше знал только сам процесс сервера (через `Descriptor`/`Reflection` API, который разбирали в теме про день 4), теперь доступно по сети кому угодно.

**Что реально произошло в демо**

Клиент отправил bidi-стрим запрос `list_services` — получил список всех сервисов на этом сервере (свой `myapp.UserService` плюс сам служебный `ServerReflection`). Затем `file_containing_symbol="myapp.UserService"` — сервер прислал сериализованный `FileDescriptorProto` (те же байты дескриптора, что генерирует protoc, только переданные по проводу, а не зашитые в скомпилированный `.pb.cc`). Клиент распарсил их через `google::protobuf::FileDescriptorProto` и вывел человекочитаемую схему — с точным различением unary/streaming методов (`stream` у `ListUsers`), хотя ни одна строчка `user_service.grpc.pb.h` не была скомпилирована в `reflection_client.cc`.

**Зачем это нужно на практике**

Отладка продакшен/staging-сервисов без пересборки клиента под конкретную версию `.proto`; инструменты вроде `grpcurl`/Postman/BloomRPC полностью полагаются на эту reflection-схему, чтобы формировать запросы к незнакомому gRPC-сервису интерактивно; также используется для service discovery в сложных инфраструктурах.

**`grpcurl` — тот же механизм, но готовым инструментом**

`grpcurl` — консольная утилита (Go), делающая ровно то же самое, что мой `reflection_client.cc`, но универсально для любого сервиса и с удобным CLI:

```bash
# установка (нужен Go)
go install github.com/fullstorydev/grpcurl/cmd/grpcurl@latest

# аналог первого запроса из демо
grpcurl -plaintext 127.0.0.1:5000 list

# аналог второго запроса
grpcurl -plaintext 127.0.0.1:5000 describe myapp.UserService

# реальный вызов метода, без единой строчки сгенерированного кода
grpcurl -plaintext -d '{"user_id": 1}' 127.0.0.1:5000 myapp.UserService/GetUser
```

**Важная деталь безопасности**

Reflection service отдаёт полную схему API кому угодно, кто может подключиться — в проде это обычно **выключают** или защищают отдельно (interceptor/mTLS, которые разбирали раньше), включая только на internal/staging окружениях, чтобы не облегчать разведку API потенциальному атакующему.
