#include <atomic>
#include <chrono>
#include <iostream>
#include <format>
#include <memory>
#include <thread>
#include <string>

#include <grpcpp/grpcpp.h>

#include "resilience_service.grpc.pb.h"

namespace {
    const std::string ADDRESS{"127.0.0.1:5000"};

    // ### SERVER ###
    std::atomic<int> g_ping_attempts{0};

    class ResilienceServiceImpl final: public myapp::ResilienceService::Service {
        grpc::Status Ping(grpc::ServerContext *context,
                    const myapp::PingRequest*,
                    myapp::PingResponse *response) override {
            const int attempt{++g_ping_attempts};
            std::cout << std::format("[server] ping, attempt #{}\n", attempt);

            if (attempt <= 2) {
                // Искусственный сбой первых двух попыток — код UNAVAILABLE считается
                // "временной проблемой", именно такие коды retry-политики обычно
                // помечают как retryable.
                return grpc::Status{grpc::StatusCode::UNAVAILABLE, "server temporarily overloaded"};
            }

            response->set_message(std::format("pong (succeeded on attempt {})", attempt));
            return grpc::Status::OK;
        }

        grpc::Status SlowCall(grpc::ServerContext *context,
                        const myapp::SlowRequest*,
                        myapp::SlowResponse*) override {
            std::cout << "[server] SlowCall starts 'work' (sleep 2 seconds)\n";
            for (int i{}; i < 20; ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                if (context->IsCancelled()) {
                    // Клиент уже отвалился по deadline — дальше работать бессмысленно.
                    std::cout << std::format("[server] discovered: client cancelled call (deadline), "
                                             "cancelling work on step {}\n", i);
                    return grpc::Status{grpc::StatusCode::CANCELLED, "cancelled by client deadline"};
                }
            }
            std::cout << "[server] SlowCall finished work\n";

            return grpc::Status::OK;
        }
    };

    void start_server() {
        ResilienceServiceImpl service;

        grpc::ServerBuilder builder;
        builder.AddListeningPort(ADDRESS, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);

        std::unique_ptr<grpc::Server> server = builder.BuildAndStart();
        std::cout << std::format("[server] listen to {}\n", ADDRESS);
        server->Wait();
    }

    // ### CLIENT ###

    // ===== 1. Deadline / timeout =====
    void demoDeadline() {
        std::cout << "\n=== Deadline: SlowCall with deadline=500ms (server will sleep 2s) ===\n";
        auto channel = grpc::CreateChannel(ADDRESS, grpc::InsecureChannelCredentials());
        auto stub = myapp::ResilienceService::NewStub(channel);

        grpc::ClientContext context;
        auto deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(500);
        context.set_deadline(deadline);

        myapp::SlowRequest request;
        myapp::SlowResponse response;
        auto start{std::chrono::steady_clock::now()};
        grpc::Status status{stub->SlowCall(&context, request, &response)};
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start).count();

        std::cout << std::format("[client] concluded in {} ms (and not in ~2000ms)\n", elapsed);
        std::cout << std::format("[client] status.ok()= {}, code= {}, {}, message= {}\n",
            status.ok(),
            static_cast<int>(status.error_code()),
            (status.error_code() == grpc::StatusCode::DEADLINE_EXCEEDED ? "DEADLINE_EXCEEDED" : "OTHER"),
            status.error_message());
    }

    // ===== 2. Retry policy через service config =====
    void demoRetry() {
        std::cout << "\n=== Retry: Ping with retry-policy (server breaks first two attempts) ===\n";
        const char* service_config_json = R"JSON(
        {
          "methodConfig": [{
            "name": [{"service": "myapp.ResilienceService", "method": "Ping"}],
            "retryPolicy": {
              "maxAttempts": 5,
              "initialBackoff": "0.1s",
              "maxBackoff": "1s",
              "backoffMultiplier": 2,
              "retryableStatusCodes": ["UNAVAILABLE"]
            }
          }]
        })JSON";

        grpc::ChannelArguments args;
        args.SetString(GRPC_ARG_SERVICE_CONFIG, service_config_json);
        args.SetInt(GRPC_ARG_ENABLE_RETRIES, 1);

        auto channel = grpc::CreateCustomChannel(
            ADDRESS, grpc::InsecureChannelCredentials(), args);
        auto stub = myapp::ResilienceService::NewStub(channel);

        grpc::ClientContext context;
        myapp::PingRequest request;
        myapp::PingResponse response;

        std::cout << "[client] will do one call stub->Ping(...)\n";
        grpc::Status status = stub->Ping(&context, request, &response);

        std::cout << "[client] status.ok()=" << status.ok() << "\n";
        if (status.ok()) {
          std::cout << "[client] answer: " << response.message()
                     << "  <- gRPC made the call again internally; our code didn't do that.\n";
        } else {
          std::cout << "[client] error: " << status.error_message() << "\n";
        }
    }

    void start_client() {
        demoDeadline();
        demoRetry();
    }

}

int main(const int argc, char *argv[]) {
    if (const std::string START_KIND{argc > 1 ? argv[1] : ""};
        START_KIND == "server") start_server();
    else if (START_KIND == "client") start_client();
    else std::cout << "BAD KIND\n";

    return 0;
}