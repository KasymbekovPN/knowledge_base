#include <iostream>
#include <format>
#include <string>
#include <memory>

#include <grpcpp/grpcpp.h>

#include "user_service.grpc.pb.h"


namespace {
    // ### SERVER ###

    // Простейшая "база данных" в памяти — для примера этого достаточно.
    class UserServiceImpl final : public myapp::UserService::Service {
        grpc::Status GetUser(grpc::ServerContext* context,
                             const myapp::UserRequest* request,
                             myapp::UserResponse* response) override {

            std::cout << std::format("[server] requesr GetUser(user_id={})\n", request->user_id());

            if (request->user_id() == 1) {
                response->set_user_id(1);
                response->set_name("Alice");
                response->set_email("alice@example.com");

                return grpc::Status::OK;
            }

            // Пользователь не найден — возвращаем ошибку через grpc::Status,
            // а не через "пустое" сообщение. Клиент должен проверять status.ok().
            return grpc::Status{grpc::StatusCode::NOT_FOUND, "user not found"};
        }
    };

    void start_server() {
        std::string address{"127.0.0.1:50070"};
        UserServiceImpl service;

        grpc::ServerBuilder builder;
        builder.AddListeningPort(address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);

        std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
        std::cout << std::format("[server] listening to {}, wait request...", address);
        // блокирует процесс — это отдельный, самостоятельный сервер
        server->Wait();
    }

    // ### CLIENT ###

    void callGetUser(myapp::UserService::Stub* stub, const int user_id) {
        myapp::UserRequest request;
        request.set_user_id(user_id);

        myapp::UserResponse response;
        grpc::ClientContext context;
        grpc::Status status{
            stub->GetUser(&context, request, &response)
        };

        if (status.ok()) {
            std::cout << std::format("[client] GetUser({}) -> {} <{}>\n",
                user_id,
                response.name(),
                response.email());
        } else {
            std::cout
                << std::format("[client] GetUser({}) -> error ", user_id)
                << status.error_code() << " "
                << status.error_message()
                << std::endl;
        }
    }

    void start_client() {
        std::string address("127.0.0.1:50070");
        auto channel = grpc::CreateChannel(address, grpc::InsecureChannelCredentials());
        std::unique_ptr<myapp::UserService::Stub> stub = myapp::UserService::NewStub(channel);

        callGetUser(stub.get(), 1);   // существующий пользователь
        callGetUser(stub.get(), 999); // несуществующий — проверяем обработку ошибки
    }

}

int main(const int argc, char *argv[]) {
    if (const std::string START_KIND{argc > 1 ? argv[1] : ""};
        START_KIND == "server") start_server();
    else if (START_KIND == "client") start_client();
    else std::cout << "BAD KIND\n";

    return 0;
}
