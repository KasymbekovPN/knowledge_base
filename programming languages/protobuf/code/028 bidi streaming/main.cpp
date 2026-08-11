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

