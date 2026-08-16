#include <iostream>
#include <format>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <grpcpp/grpcpp.h>

#include <grpcpp/ext/proto_server_reflection_plugin.h>

#include "task_service.grpc.pb.h"

#include "params.hpp"
#include "read_file.hpp"

namespace {

    // ===== In-memory хранилище задач =====
    // grpc::Server по умолчанию многопоточный (пул потоков на входящие RPC),
    // поэтому доступ к общему состоянию защищаем мьютексом.
    class TaskStore {
    public:
        myapp::tasks::v1::Task create(const std::string &title,
                                      const std::string& description) {
            std::lock_guard<std::mutex> lock{mutex_};
            myapp::tasks::v1::Task t;
            t.set_id(next_id_++);
            t.set_title(title);
            t.set_description(description);
            t.set_done(false);
            tasks_.push_back(t);

            return t;
        }

        bool get(const int32_t id, myapp::tasks::v1::Task* out) {
            std::lock_guard<std::mutex> lock{mutex_};
            return std::ranges::any_of(tasks_, [&id, &out](const myapp::tasks::v1::Task& t) {
                if (t.id() == id) {
                    *out = t;
                    return true;
                }
                return false;
            });
        }

        std::vector<myapp::tasks::v1::Task> all() {
            std::lock_guard<std::mutex> lock{mutex_};
            return tasks_;
        }

    private:
        std::mutex mutex_;
        std::vector<myapp::tasks::v1::Task> tasks_;
        int32_t next_id_{1};
    };

    class TaskServiceImpl final: public myapp::tasks::v1::TaskService::Service {
    public:
        grpc::Status CreateTask(grpc::ServerContext *context, const myapp::tasks::v1::CreateTaskRequest *request,
            myapp::tasks::v1::Task *response) override {
            if (request->title().empty()) {
                // Базовая обработка ошибок: невалидный ввод -> INVALID_ARGUMENT,
                // а не молчаливое создание "пустой" задачи.
                return grpc::Status{grpc::StatusCode::INVALID_ARGUMENT, "title must not by empty"};
            }

            *response = store_.create(request->title(), request->description());
            std::cout << std::format("[server] CreateTask -> id= {}, '{}'\n",
                response->id(), response->title());

            return grpc::Status::OK;
        }

        grpc::Status GetTask(grpc::ServerContext *context, const myapp::tasks::v1::TaskId *request,
            myapp::tasks::v1::Task *response) override {
            if (store_.get(request->id(), response)) {
                std::cout << std::format("[server] GetTask({}) -> found\n", request->id());

                return grpc::Status::OK;
            }

            std::cout << std::format("[server] GetTask({}) -> not found\n", request->id());
            return grpc::Status{
                grpc::StatusCode::NOT_FOUND,
                std::format("task {} not found", request->id())
            };
        }

        grpc::Status ListTasks(grpc::ServerContext *context, const myapp::tasks::v1::ListTasksRequest *request,
            grpc::ServerWriter<myapp::tasks::v1::Task> *writer) override {
            const auto tasks{store_.all()};
            std::cout << std::format("[server] ListTasks -> streaming {} tasks\n", tasks.size());
            for (const auto& t: tasks) writer->Write(t);

            return grpc::Status::OK;
        }

        grpc::Status BulkCreateTasks(grpc::ServerContext *context,
            grpc::ServerReader<myapp::tasks::v1::CreateTaskRequest> *reader,
            myapp::tasks::v1::BulkCreateSummary *summary) override {

            myapp::tasks::v1::CreateTaskRequest request;
            int count{};
            while (reader->Read(&request)) {
                if (!request.title().empty()) {
                    store_.create(request.title(), request.description());
                    ++count;
                }
            }

            summary->set_created_count(count);
            std::cout << std::format("[server] BulkCreateTask -> created {} tasks\n", count);

            return grpc::Status::OK;
        }
    private:
        TaskStore store_;
    };

}

int main(const int argc, char *argv[]) {
    const std::string mode{params::get_mode(argc, argv)};

    TaskServiceImpl service;
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();

    grpc::ServerBuilder builder;

    std::shared_ptr<grpc::ServerCredentials> creds;
    if (mode == params::MODE_INSECURE) {
        creds = grpc::InsecureServerCredentials();
    } else {
        grpc::SslServerCredentialsOptions ssl_opts{
            mode == params::MODE_MTLS
            ? GRPC_SSL_REQUEST_AND_REQUIRE_CLIENT_CERTIFICATE_AND_VERIFY
            : GRPC_SSL_DONT_REQUEST_CLIENT_CERTIFICATE
        };
        grpc::SslServerCredentialsOptions::PemKeyCertPair cert_pair;
        cert_pair.private_key = myapp::read_file("certs/server.key");
        cert_pair.cert_chain = myapp::read_file("certs/server.crt");
        ssl_opts.pem_key_cert_pairs.push_back(cert_pair);
        if (mode == params::MODE_MTLS) {
            ssl_opts.pem_root_certs = myapp::read_file("certs/ca.crt");
        }
        creds = grpc::SslServerCredentials(ssl_opts);
    }

    builder.AddListeningPort(params::ADDRESS, creds);
    builder.RegisterService(&service);

    const std::unique_ptr<grpc::Server> server{builder.BuildAndStart()};
    std::cout << std::format("[server] listen to {}, mode= {}, reflection ON\n", params::ADDRESS, mode);
    server->Wait();

    return 0;
}
