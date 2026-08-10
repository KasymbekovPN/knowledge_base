#include <chrono>
#include <iostream>
#include <format>
#include <memory>
#include <thread>

#include <grpcpp/grpcpp.h>

#include "log_service.grpc.pb.h"

namespace {
    // ### SERVER ###

    std::string now_ms() {
        const auto now{std::chrono::system_clock::now().time_since_epoch()};
        const auto ms{std::chrono::duration_cast<std::chrono::milliseconds>(now).count()};

        return std::to_string(ms % 1'000'000);
    }

    class LogServiceImpl final: public myapp::LogService::Service {
        // ===== Server streaming: сервер САМ решает, когда писать в поток =====
        grpc::Status StreamLogs(grpc::ServerContext* context,
                                const myapp::LogQuery* request,
                                grpc::ServerWriter<myapp::LogEntry>* writer) override {
            std::cout << std::format("[t= {}] [server] log streaming start for '{}'\n",
                now_ms(),
                request->service_name());
            const char* messages[] = {
                "service started",
                "connected to db",
                "handling request #1",
                "handling request #2",
                "graceful shutdown"
            };

            for (const char* msg: messages) {
                // имитируем, что лог реально появляется "по мере поступления",
                // а не берётся из уже готового списка мгновенно
                std::this_thread::sleep_for(std::chrono::milliseconds(400));

                myapp::LogEntry entry;
                entry.set_level("INFO");
                entry.set_message(msg);
                std::cout << std::format("[t= {}] [server] -> write into stream '{}'\n", now_ms(), msg);
                writer->Write(entry);
            }

            std::cout << std::format("[t= {}] [server] log stream done\n", now_ms());

            return grpc::Status::OK;
        }

        // ===== Client streaming: сервер читает, пока клиент не закончит =====
        grpc::Status UploadLogs(grpc::ServerContext* context,
                                grpc::ServerReader<myapp::LogEntry>* reader,
                                myapp::UploadSummary* summary) override {
            myapp::LogEntry entry;
            int count{};
            while (reader->Read(&entry)) {
                ++count;
                std::cout << std::format("[t= {}] [server] <- take log #{}: [{}] {}\n",
                    now_ms(),
                    count,
                    entry.level(),
                    entry.message());
            }

            summary->set_accepted_count(count);
            summary->set_status("OK");
            std::cout << std::format("[t= {}] [server] loading done, taken {} entry\n", now_ms(), count);

            return grpc::Status::OK;
        }
    };

    void start_server() {
        std::string address{"127.0.0.1:5081"};
        LogServiceImpl service;

        grpc::ServerBuilder builder;
        builder.AddListeningPort(address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);

        std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
        std::cout << std::format("[server] listen to {}\n", address);
        server->Wait();
    }

    // ### CLIENT ###

    // ===== Server streaming: клиент читает по мере поступления, НЕ ждёт всё сразу =====
    void demoStreamLogs(myapp::LogService::Stub* stub) {
        std::cout << "\n=== Server streaming: StreamLogs ===\n";
        grpc::ClientContext context;
        myapp::LogQuery query;
        query.set_service_name("payments");

        std::unique_ptr<grpc::ClientReader<myapp::LogEntry>> reader{
            stub->StreamLogs(&context, query)
        };

        myapp::LogEntry entry;
        while (reader->Read(&entry)) {
            // именно этот момент времени доказывает, что данные пришли ПОСТЕПЕННО,
            // а не все разом в конце вызова
            std::cout << std::format("[t= {}] [client] log received: [{}] '{}'\n",
                now_ms(),
                entry.level(),
                entry.message());
        }

        std::cout << std::format("[client] StreamLogs done, ok= {}\n", reader->Finish().ok());
    }

    // ===== Client streaming: клиент сам решает темп отправки =====
    void demoUploadLogs(myapp::LogService::Stub* stub) {
        std::cout << "\n=== Client streaming: UploadLogs ===\n";
        grpc::ClientContext context;
        myapp::UploadSummary summary;

        std::unique_ptr<grpc::ClientWriter<myapp::LogEntry>> writer{
            stub->UploadLogs(&context, &summary)
        };

        const char* messages[] = {
            "cache miss",
            "retrying request",
            "recovered"
        };
        for (const char* msg: messages) {
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            myapp::LogEntry entry;
            entry.set_level("WARN");
            entry.set_message(msg);
            std::cout << std::format("[t= {}] [client] -> sending: '{}'\n", now_ms(), msg);
            writer->Write(entry);
        }

        writer->WritesDone();
        grpc::Status status = writer->Finish();
        std::cout << std::format("[client] UploadLogs done, ok= {}, received= {}, status={}\n",
            status.ok(),
            summary.accepted_count(),
            summary.status());
    }

    void start_client() {
        std::string address{"127.0.0.1:5081"};
        auto channel = grpc::CreateChannel(address, grpc::InsecureChannelCredentials());
        std::unique_ptr<myapp::LogService::Stub> stub = myapp::LogService::NewStub(channel);

        demoStreamLogs(stub.get());
        demoUploadLogs(stub.get());
    }
}

int main(const int argc, char *argv[]) {
    if (const std::string START_KIND{argc > 1 ? argv[1] : ""};
        START_KIND == "server") start_server();
    else if (START_KIND == "client") start_client();
    else std::cout << "BAD KIND\n";

    return 0;
}
