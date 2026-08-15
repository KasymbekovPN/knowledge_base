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
