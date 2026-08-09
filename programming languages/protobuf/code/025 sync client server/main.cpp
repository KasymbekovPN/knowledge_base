// #include <chrono>
// #include <iostream>
// #include <memory>
// #include <thread>
// #include <grpcpp/grpcpp.h>
// #include "order_service.grpc.pb.h"
//
// using grpc::Server;
// using grpc::ServerBuilder;
// using grpc::ServerContext;
// using grpc::ServerReader;
// using grpc::ServerReaderWriter;
// using grpc::ServerWriter;
// using grpc::Status;
// using grpc::ClientContext;
// using grpc::ClientReader;
// using grpc::ClientWriter;
// using grpc::ClientReaderWriter;
// using grpc::Channel;
//
// using myapp::OrderRequest;
// using myapp::OrderResponse;
// using myapp::OrderUpdate;
// using myapp::Ack;
// using myapp::OrderService;
//
// // ===================== СЕРВЕР =====================
// // Реализуем Service — базовый класс уже даёт дефолтную реализацию
// // (UNIMPLEMENTED) для всех методов, переопределяем только нужные.
// class OrderServiceImpl final : public OrderService::Service {
//   Status GetOrder(ServerContext* context, const OrderRequest* request,
//                   OrderResponse* response) override {
//     response->set_order_id(request->order_id());
//     response->set_status("PAID");
//     return Status::OK;
//   }
//
//   Status WatchOrderStatus(ServerContext* context, const OrderRequest* request,
//                            ServerWriter<OrderResponse>* writer) override {
//     const char* statuses[] = {"PENDING", "PAID", "SHIPPED", "DELIVERED"};
//     for (const char* s : statuses) {
//       OrderResponse resp;
//       resp.set_order_id(request->order_id());
//       resp.set_status(s);
//       writer->Write(resp);  // сервер сам решает, сколько раз писать в поток
//     }
//     return Status::OK;
//   }
//
//   Status BatchUpdateOrder(ServerContext* context, ServerReader<OrderUpdate>* reader,
//                            Ack* response) override {
//     OrderUpdate update;
//     int count = 0;
//     while (reader->Read(&update)) {  // читаем, пока клиент не завершит поток
//       ++count;
//       std::cout << "  [сервер] получено обновление #" << count << ": "
//                  << update.field() << "=" << update.value() << "\n";
//     }
//     response->set_ok(count > 0);
//     return Status::OK;
//   }
//
//   Status SyncOrders(ServerContext* context,
//                      ServerReaderWriter<OrderResponse, OrderUpdate>* stream) override {
//     OrderUpdate update;
//     while (stream->Read(&update)) {  // читаем один запрос клиента...
//       OrderResponse resp;
//       resp.set_order_id(update.order_id());
//       resp.set_status("ACK:" + update.field());
//       stream->Write(resp);  // ...и сразу отвечаем — в рамках одного соединения
//     }
//     return Status::OK;
//   }
// };
//
// void RunServer(std::unique_ptr<Server>* out_server, const std::string& address) {
//   OrderServiceImpl service;
//   ServerBuilder builder;
//   builder.AddListeningPort(address, grpc::InsecureServerCredentials());
//   builder.RegisterService(&service);
//   *out_server = builder.BuildAndStart();
//   std::cout << "[сервер] слушает на " << address << "\n";
//   (*out_server)->Wait();  // блокирует поток до Shutdown()
// }
//
// // ===================== КЛИЕНТ =====================
// void RunClient(const std::string& address) {
//   auto channel = grpc::CreateChannel(address, grpc::InsecureChannelCredentials());
//   std::unique_ptr<OrderService::Stub> stub = OrderService::NewStub(channel);
//
//   // --- 1. Unary ---
//   {
//     ClientContext context;
//     OrderRequest req;
//     req.set_order_id(42);
//     OrderResponse resp;
//     Status status = stub->GetOrder(&context, req, &resp);
//     std::cout << "\n[клиент] Unary GetOrder: ok=" << status.ok()
//                << ", status=" << resp.status() << "\n";
//   }
//
//   // --- 2. Server streaming ---
//   {
//     ClientContext context;
//     OrderRequest req;
//     req.set_order_id(42);
//     std::unique_ptr<ClientReader<OrderResponse>> reader =
//         stub->WatchOrderStatus(&context, req);
//     std::cout << "[клиент] Server streaming WatchOrderStatus:\n";
//     OrderResponse resp;
//     while (reader->Read(&resp)) {
//       std::cout << "  <- " << resp.status() << "\n";
//     }
//     Status status = reader->Finish();
//     std::cout << "  Finish ok=" << status.ok() << "\n";
//   }
//
//   // --- 3. Client streaming ---
//   {
//     ClientContext context;
//     Ack ack;
//     std::unique_ptr<ClientWriter<OrderUpdate>> writer =
//         stub->BatchUpdateOrder(&context, &ack);
//     std::cout << "[клиент] Client streaming BatchUpdateOrder:\n";
//     for (const std::string& field : {"priority", "note"}) {
//       OrderUpdate u;
//       u.set_order_id(42);
//       u.set_field(field);
//       u.set_value("x");
//       writer->Write(u);
//       std::cout << "  -> отправлено: " << field << "\n";
//     }
//     writer->WritesDone();
//     Status status = writer->Finish();
//     std::cout << "  Finish ok=" << status.ok() << ", ack.ok=" << ack.ok() << "\n";
//   }
//
//   // --- 4. Bidi streaming ---
//   {
//     ClientContext context;
//     std::unique_ptr<ClientReaderWriter<OrderUpdate, OrderResponse>> stream =
//         stub->SyncOrders(&context);
//     std::cout << "[клиент] Bidi streaming SyncOrders:\n";
//     for (const std::string& field : {"color", "size"}) {
//       OrderUpdate u;
//       u.set_order_id(42);
//       u.set_field(field);
//       stream->Write(u);
//       OrderResponse resp;
//       stream->Read(&resp);  // синхронный ping-pong: пишем и сразу читаем ответ
//       std::cout << "  <-> " << field << " => " << resp.status() << "\n";
//     }
//     stream->WritesDone();
//     Status status = stream->Finish();
//     std::cout << "  Finish ok=" << status.ok() << "\n";
//   }
// }
//
// int main() {
//   const std::string address = "127.0.0.1:50061";
//   std::unique_ptr<Server> server;
//
//   std::thread server_thread([&]() { RunServer(&server, address); });
//   std::this_thread::sleep_for(std::chrono::milliseconds(300));  // дать серверу стартовать
//
//   RunClient(address);
//
//   server->Shutdown();
//   server_thread.join();
//   std::cout << "\n[сервер] остановлен\n";
//   return 0;
// }