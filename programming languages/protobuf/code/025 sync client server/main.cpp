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
