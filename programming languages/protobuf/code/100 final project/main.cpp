#inclide <iostream>
#include <format>
#include <fstream>
#include <sstream>
#include <memory>
#include <string>
#include <mutex>
#include <vector>

namespace {
    std::string readFile(const std::string& path) {
        std::ifstream f{path};
        std::stringstream ss;
        ss << f.rdbuf();

        return ss.str();
    }
}

namespace server {

    namespace {
        // ===== In-memory хранилище задач =====
        // grpc::Server по умолчанию многопоточный (пул потоков на входящие RPC),
        // поэтому доступ к общему состоянию защищаем мьютексом.
        class TaskStore {
        public:
            Task create(const std::string& title, const std::string& description) {
                std::lock_guard<std::mutex> lock{mutex_};

                //     Task t;
                //     t.set_id(next_id_++);
                //     t.set_title(title);
                //     t.set_description(description);
                //     t.set_done(false);
                //     tasks_.push_back(t);
                //     return t;
            }

            bool get(const int32_t id, Task* out) {
                //     std::lock_guard<std::mutex> lock(mutex_);
                //     for (const auto& t : tasks_) {
                //       if (t.id() == id) {
                //         *out = t;
                //         return true;
                //       }
                //     }
                //     return false;
            }

            std::vector<Task> all() {
                //     std::lock_guard<std::mutex> lock(mutex_);
                //     return tasks_;
            }

        private:
            std::mutex mutex_;
            std::vector<Task> tasks_;
            int32_t next_id_{0};
        };
    }
//   std::mutex mutex_;
//   std::vector<Task> tasks_;
//   int32_t next_id_ = 1;
// };
//
// class TaskServiceImpl final : public TaskService::Service {
//  public:
//   Status CreateTask(ServerContext*, const CreateTaskRequest* request,
//                      Task* response) override {
//     if (request->title().empty()) {
//       // Базовая обработка ошибок: невалидный ввод -> INVALID_ARGUMENT,
//       // а не молчаливое создание "пустой" задачи.
//       return Status(grpc::StatusCode::INVALID_ARGUMENT, "title must not be empty");
//     }
//     *response = store_.Create(request->title(), request->description());
//     std::cout << "[сервер] CreateTask -> id=" << response->id()
//                << " \"" << response->title() << "\"\n";
//     return Status::OK;
//   }
//
//   Status GetTask(ServerContext*, const TaskId* request, Task* response) override {
//     if (store_.Get(request->id(), response)) {
//       std::cout << "[сервер] GetTask(" << request->id() << ") -> найдено\n";
//       return Status::OK;
//     }
//     std::cout << "[сервер] GetTask(" << request->id() << ") -> NOT_FOUND\n";
//     return Status(grpc::StatusCode::NOT_FOUND,
//                   "task " + std::to_string(request->id()) + " not found");
//   }
//
//   Status ListTasks(ServerContext*, const ListTasksRequest*,
//                     ServerWriter<Task>* writer) override {
//     auto tasks = store_.All();
//     std::cout << "[сервер] ListTasks -> стримит " << tasks.size() << " задач\n";
//     for (const auto& t : tasks) {
//       writer->Write(t);
//     }
//     return Status::OK;
//   }
//
//   Status BulkCreateTasks(ServerContext*, ServerReader<CreateTaskRequest>* reader,
//                           BulkCreateSummary* summary) override {
//     CreateTaskRequest req;
//     int count = 0;
//     while (reader->Read(&req)) {
//       if (!req.title().empty()) {
//         store_.Create(req.title(), req.description());
//         ++count;
//       }
//     }
//     summary->set_created_count(count);
//     std::cout << "[сервер] BulkCreateTasks -> создано " << count << " задач\n";
//     return Status::OK;
//   }
//
//  private:
//   TaskStore store_;
// };
//
// int main(int argc, char** argv) {
//   std::string mode = (argc > 1) ? argv[1] : "insecure";  // insecure | tls | mtls
//   std::string address("127.0.0.1:50200");
//
//   TaskServiceImpl service;
//   grpc::reflection::InitProtoReflectionServerBuilderPlugin();
//
//   ServerBuilder builder;
//
//   std::shared_ptr<grpc::ServerCredentials> creds;
//   if (mode == "insecure") {
//     creds = grpc::InsecureServerCredentials();
//   } else {
//     grpc::SslServerCredentialsOptions ssl_opts(
//         mode == "mtls" ? GRPC_SSL_REQUEST_AND_REQUIRE_CLIENT_CERTIFICATE_AND_VERIFY
//                         : GRPC_SSL_DONT_REQUEST_CLIENT_CERTIFICATE);
//     grpc::SslServerCredentialsOptions::PemKeyCertPair key_cert_pair;
//     key_cert_pair.private_key = ReadFile("/tmp/certs/server.key");
//     key_cert_pair.cert_chain = ReadFile("/tmp/certs/server.crt");
//     ssl_opts.pem_key_cert_pairs.push_back(key_cert_pair);
//     if (mode == "mtls") {
//       ssl_opts.pem_root_certs = ReadFile("/tmp/certs/ca.crt");
//     }
//     creds = grpc::SslServerCredentials(ssl_opts);
//   }
//
//   builder.AddListeningPort(address, creds);
//   builder.RegisterService(&service);
//
//   std::unique_ptr<Server> server = builder.BuildAndStart();
//   std::cout << "[сервер] слушает на " << address << ", режим=" << mode
//              << ", reflection включена\n";
//   server->Wait();
//   return 0;
// }

    void start() {}
}

namespace client {
    void start() {}

    // void DemoCreateTask(TaskService::Stub* stub, const std::string& title) {
//   ClientContext context;
//   CreateTaskRequest req;
//   req.set_title(title);
//   req.set_description("описание для " + title);
//   Task response;
//
//   Status status = stub->CreateTask(&context, req, &response);
//   std::cout << "[клиент] CreateTask(\"" << title << "\") -> ok=" << status.ok();
//   if (status.ok()) std::cout << ", id=" << response.id();
//   std::cout << "\n";
// }
//
// void DemoGetTask(TaskService::Stub* stub, int id) {
//   ClientContext context;
//   TaskId req;
//   req.set_id(id);
//   Task response;
//
//   Status status = stub->GetTask(&context, req, &response);
//   std::cout << "[клиент] GetTask(" << id << ") -> ok=" << status.ok();
//   if (status.ok()) {
//     std::cout << ", title=\"" << response.title() << "\"";
//   } else {
//     // Обработка ошибки: различаем код, а не просто "что-то пошло не так"
//     std::cout << ", код=" << status.error_code()
//                << " (" << (status.error_code() == grpc::StatusCode::NOT_FOUND
//                                ? "NOT_FOUND" : "другой")
//                << "), сообщение=\"" << status.error_message() << "\"";
//   }
//   std::cout << "\n";
// }
//
// void DemoListTasks(TaskService::Stub* stub) {
//   ClientContext context;
//   ListTasksRequest req;
//   std::unique_ptr<ClientReader<Task>> reader = stub->ListTasks(&context, req);
//
//   std::cout << "[клиент] ListTasks (server streaming):\n";
//   Task task;
//   while (reader->Read(&task)) {
//     std::cout << "  <- id=" << task.id() << ", \"" << task.title() << "\"\n";
//   }
//   Status status = reader->Finish();
//   std::cout << "[клиент] ListTasks завершён, ok=" << status.ok() << "\n";
// }
//
// void DemoBulkCreateTasks(TaskService::Stub* stub) {
//   ClientContext context;
//   BulkCreateSummary summary;
//   std::unique_ptr<ClientWriter<CreateTaskRequest>> writer =
//       stub->BulkCreateTasks(&context, &summary);
//
//   std::cout << "[клиент] BulkCreateTasks (client streaming):\n";
//   for (const std::string& title : {"bulk-1", "bulk-2", "bulk-3"}) {
//     CreateTaskRequest req;
//     req.set_title(title);
//     writer->Write(req);
//     std::cout << "  -> отправлено: " << title << "\n";
//   }
//   writer->WritesDone();
//   Status status = writer->Finish();
//   std::cout << "[клиент] BulkCreateTasks завершён, ok=" << status.ok()
//              << ", создано=" << summary.created_count() << "\n";
// }
//
// int main(int argc, char** argv) {
//   std::string mode = (argc > 1) ? argv[1] : "insecure";       // insecure | tls | mtls
//   bool send_client_cert = (argc > 2 && std::string(argv[2]) == "with-cert");
//
//   std::string address("127.0.0.1:50200");
//   std::shared_ptr<grpc::ChannelCredentials> creds;
//   grpc::ChannelArguments args;
//
//   if (mode == "insecure") {
//     creds = grpc::InsecureChannelCredentials();
//   } else {
//     grpc::SslCredentialsOptions ssl_opts;
//     ssl_opts.pem_root_certs = ReadFile("/tmp/certs/ca.crt");
//     if (mode == "mtls" && send_client_cert) {
//       ssl_opts.pem_private_key = ReadFile("/tmp/certs/client.key");
//       ssl_opts.pem_cert_chain = ReadFile("/tmp/certs/client.crt");
//     }
//     args.SetSslTargetNameOverride("localhost");
//     creds = grpc::SslCredentials(ssl_opts);
//   }
//
//   auto channel = grpc::CreateCustomChannel(address, creds, args);
//   std::unique_ptr<TaskService::Stub> stub = TaskService::NewStub(channel);
//
//   std::cout << "=== режим=" << mode
//              << (mode == "mtls" ? (send_client_cert ? " (с клиентским сертификатом)"
//                                                      : " (БЕЗ клиентского сертификата)")
//                                  : "")
//              << " ===\n";
//
//   DemoCreateTask(stub.get(), "написать демо");
//   DemoCreateTask(stub.get(), "прогнать тесты");
//   DemoGetTask(stub.get(), 1);     // существующая задача
//   DemoGetTask(stub.get(), 999);   // несуществующая -> NOT_FOUND
//   DemoBulkCreateTasks(stub.get());
//   DemoListTasks(stub.get());
//
//   return 0;
// }

}

// #include <grpcpp/grpcpp.h>
// #include <grpcpp/ext/proto_server_reflection_plugin.h>
// #include "task_service.grpc.pb.h"


// #include <grpcpp/grpcpp.h>
// #include "task_service.grpc.pb.h"

//< server

//
// using grpc::Server;
// using grpc::ServerBuilder;
// using grpc::ServerContext;
// using grpc::ServerReader;
// using grpc::ServerWriter;
// using grpc::Status;
// using myapp::tasks::v1::BulkCreateSummary;
// using myapp::tasks::v1::CreateTaskRequest;
// using myapp::tasks::v1::ListTasksRequest;
// using myapp::tasks::v1::Task;
// using myapp::tasks::v1::TaskId;
// using myapp::tasks::v1::TaskService;
//

//


//< client

//
// using grpc::ClientContext;
// using grpc::ClientReader;
// using grpc::ClientWriter;
// using grpc::Status;
// using myapp::tasks::v1::BulkCreateSummary;
// using myapp::tasks::v1::CreateTaskRequest;
// using myapp::tasks::v1::ListTasksRequest;
// using myapp::tasks::v1::Task;
// using myapp::tasks::v1::TaskId;
// using myapp::tasks::v1::TaskService;
//



int main(const int argc, char *argv[]) {
    if (const std::string type{argc > 1 ? argv[1] : ""};
        type == "server") server::start();
    else if (type == "client") client::start();
    else std::cout << "BAD TYPE\n";

    return 0;
}
