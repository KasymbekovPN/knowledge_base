
namespace {
    //< server with interceptor
// #include <iostream>
// #include <memory>
// #include <string>
// #include <grpcpp/grpcpp.h>
// #include "user_service.grpc.pb.h"
// #include "interceptors.h"
//
//     using grpc::Server;
//     using grpc::ServerBuilder;
//     using grpc::ServerContext;
//     using grpc::Status;
//     using myapp::UserRequest;
//     using myapp::UserResponse;
//     using myapp::UserService;
//
//     class UserServiceImpl final : public UserService::Service {
//         Status GetUser(ServerContext* context, const UserRequest* request,
//                        UserResponse* response) override {
//             // Обрати внимание: сама бизнес-логика ничего не знает про auth —
//             // проверка токена целиком вынесена в interceptor.
//             std::cout << "[handler] GetUser(user_id=" << request->user_id() << ")\n";
//             response->set_user_id(1);
//             response->set_name("Alice");
//             response->set_email("alice@example.com");
//             return Status::OK;
//         }
//     };
//
//     int main() {
//         std::string address("127.0.0.1:50099");
//         UserServiceImpl service;
//
//         ServerBuilder builder;
//         builder.AddListeningPort(address, grpc::InsecureServerCredentials());
//         builder.RegisterService(&service);
//
//         std::vector<std::unique_ptr<grpc::experimental::ServerInterceptorFactoryInterface>>
//             creators;
//         creators.push_back(std::make_unique<ServerAuthLoggingInterceptorFactory>());
//         builder.experimental().SetInterceptorCreators(std::move(creators));
//
//         std::unique_ptr<Server> server = builder.BuildAndStart();
//         std::cout << "[сервер] слушает на " << address << " (с auth-интерцептором)\n";
//         server->Wait();
//         return 0;
//     }

    //< client with interceptor
// #include <iostream>
// #include <memory>
// #include <string>
// #include <grpcpp/grpcpp.h>
// #include "user_service.grpc.pb.h"
// #include "interceptors.h"
//
//     using grpc::ChannelArguments;
//     using grpc::ClientContext;
//     using grpc::Status;
//     using myapp::UserRequest;
//     using myapp::UserResponse;
//     using myapp::UserService;
//
//     int main() {
//         std::string address("127.0.0.1:50099");
//
//         std::vector<std::unique_ptr<grpc::experimental::ClientInterceptorFactoryInterface>>
//             creators;
//         creators.push_back(std::make_unique<ClientAuthLoggingInterceptorFactory>());
//
//         auto channel = grpc::experimental::CreateCustomChannelWithInterceptors(
//             address, grpc::InsecureChannelCredentials(), ChannelArguments(),
//             std::move(creators));
//         std::unique_ptr<UserService::Stub> stub = UserService::NewStub(channel);
//
//         // Вызов выглядит как обычно — ни намёка на auth-логику здесь нет,
//         // весь auth/логирование прилетает "снаружи" через interceptor.
//         UserRequest request;
//         request.set_user_id(1);
//         UserResponse response;
//         ClientContext context;
//
//         std::cout << "[клиент] вызываю GetUser(...)\n";
//         Status status = stub->GetUser(&context, request, &response);
//
//         std::cout << "[клиент] status.ok()=" << status.ok();
//         if (status.ok()) {
//             std::cout << ", имя=" << response.name();
//         } else {
//             std::cout << ", ошибка=" << status.error_message();
//         }
//         std::cout << "\n";
//
//         return 0;
//     }

    //< client without interceptor
// #include <iostream>
// #include <memory>
// #include <string>
// #include <grpcpp/grpcpp.h>
// #include "user_service.grpc.pb.h"
//
//     using grpc::ClientContext;
//     using grpc::Status;
//     using myapp::UserRequest;
//     using myapp::UserResponse;
//     using myapp::UserService;
//
//     int main() {
//         std::string address("127.0.0.1:50099");
//         // Обычный канал БЕЗ клиентского интерцептора — значит, без auth-токена.
//         auto channel = grpc::CreateChannel(address, grpc::InsecureChannelCredentials());
//         std::unique_ptr<UserService::Stub> stub = UserService::NewStub(channel);
//
//         UserRequest request;
//         request.set_user_id(1);
//         UserResponse response;
//         ClientContext context;
//
//         std::cout << "[клиент без токена] вызываю GetUser(...)\n";
//         Status status = stub->GetUser(&context, request, &response);
//
//         std::cout << "[клиент без токена] status.ok()=" << status.ok();
//         if (status.ok()) {
//             std::cout << ", имя=" << response.name();
//         } else {
//             std::cout << ", код=" << status.error_code()
//                        << ", ошибка=" << status.error_message();
//         }
//         std::cout << "\n";
//
//         return 0;
//     }
}


int main(const int argc, char *argv[]) {
    //<
    // if (const std::string START_KIND{argc > 1 ? argv[1] : ""};
    //     START_KIND == "server") start_server();
    // else if (START_KIND == "client") start_client();
    // else std::cout << "BAD KIND\n";

    return 0;
}