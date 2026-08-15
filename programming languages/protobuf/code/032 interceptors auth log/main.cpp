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