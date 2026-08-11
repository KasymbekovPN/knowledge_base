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

