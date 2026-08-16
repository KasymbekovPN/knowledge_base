#include <iostream>
#include <format>
#include <memory>

#include <grpcpp/grpcpp.h>

#include "task_service.grpc.pb.h"

#include "params.hpp"
#include "read_file.hpp"

namespace {
    void demoCreateTask(myapp::tasks::v1::TaskService::Stub* stub,
                        const std::string& title) {
        grpc::ClientContext context;
        myapp::tasks::v1::CreateTaskRequest request;
        request.set_title(title);
        request.set_description(std::format("description for {}\n", title));
        myapp::tasks::v1::Task response;

        const auto status{stub->CreateTask(&context, request, &response)};
        std::cout << std::format("[client] CreateTask('{}') -> ok= {}", title, status.ok());
        if (status.ok()) std::cout << std::format(", id= {}", response.id());
        std::cout << '\n';
    }

    void demoGetTask(myapp::tasks::v1::TaskService::Stub* stub, const int id) {
        grpc::ClientContext context;
        myapp::tasks::v1::TaskId request;
        request.set_id(id);
        myapp::tasks::v1::Task response;

        const auto status{stub->GetTask(&context, request, &response)};
        std::cout << std::format("[client] GetTask('{}') -> ok= {}", id, status.ok());
        if (status.ok()) {
            std::cout << std::format(", title = '{}'", response.title());
        } else {
            // Обработка ошибки: различаем код, а не просто "что-то пошло не так"
            std::cout << std::format(", code= {}, ({}), message= {}",
                static_cast<int>(status.error_code()),
                (status.error_code() == grpc::StatusCode::NOT_FOUND
                    ? "NOT FOUND"
                    : "OTHER"),
                status.error_message());
        }
        std::cout << '\n';
    }

    void demoListTasks(myapp::tasks::v1::TaskService::Stub* stub) {
        grpc::ClientContext context;
        const myapp::tasks::v1::ListTasksRequest request;
        std::unique_ptr<grpc::ClientReader<myapp::tasks::v1::Task>> reader{
            stub->ListTasks(&context, request)
        };

        std::cout << "[client] ListTasks(server streaming):";
        myapp::tasks::v1::Task task;
        while (reader->Read(&task)) {
            std::cout << std::format(" <- id= {}, '{}'\n", task.id(), task.title());
        }

        std::cout << std::format("[client] ListTasks completed, ok= {}\n", reader->Finish().ok());
    }

    void demoBulkCreateTasks(myapp::tasks::v1::TaskService::Stub* stub) {
        grpc::ClientContext context;
        myapp::tasks::v1::BulkCreateSummary summary;
        const std::unique_ptr<grpc::ClientWriter<myapp::tasks::v1::CreateTaskRequest>> writer{
            stub->BulkCreateTasks(&context, &summary)
        };

        std::cout << std::format("[client] BulkCreateTasks (client streaming):\n");
        for (const std::string& title : {"bulk-1", "bulk-2", "bulk-3"}) {
            myapp::tasks::v1::CreateTaskRequest request;
            request.set_title(title);
            writer->Write(request);
            std::cout << std::format("  -> sent: '{}'\n", title);
        }

        writer->WritesDone();
        std::cout << std::format("[client] BulkCreateTasks, ok= {}, created= {}\n",
            writer->Finish().ok(),
            summary.created_count());
    }

}

int main(const int argc, char *argv[]) {
    const std::string mode{params::get_mode(argc, argv)};
    const bool send_client_cert{params::with_cert(argc, argv)};

    std::shared_ptr<grpc::ChannelCredentials> creds;
    grpc::ChannelArguments args;

    if (mode == params::MODE_INSECURE) {
        creds = grpc::InsecureChannelCredentials();
    } else {
        grpc::SslCredentialsOptions ssl_opts;
        ssl_opts.pem_root_certs = myapp::read_file("certs/ca.crt");
        if (mode == params::MODE_MTLS && send_client_cert) {
            ssl_opts.pem_private_key = myapp::read_file("certs/client.key");
            ssl_opts.pem_cert_chain = myapp::read_file("certs/client.crt");
        }
        args.SetSslTargetNameOverride("localhost");
        creds = grpc::SslCredentials(ssl_opts);
    }

    const auto channel{grpc::CreateCustomChannel(params::ADDRESS, creds, args)};
    std::unique_ptr<myapp::tasks::v1::TaskService::Stub> stub{
        myapp::tasks::v1::TaskService::NewStub(channel)
    };

    std::cout << std::format("=== mode = {} ===\n",
        (send_client_cert ? "CRT" : "NO CRT"));

    demoCreateTask(stub.get(), "write demo");
    demoCreateTask(stub.get(), "make tests");
    demoGetTask(stub.get(), 1);
    demoGetTask(stub.get(), 999);
    demoBulkCreateTasks(stub.get());
    demoListTasks(stub.get());

    return 0;
}
