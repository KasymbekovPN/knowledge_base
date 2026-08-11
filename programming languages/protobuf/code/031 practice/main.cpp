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