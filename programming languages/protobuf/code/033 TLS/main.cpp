#include <fstream>
#include <sstream>
#include <iostream>
#include <format>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "user_service.grpc.pb.h"

const static std::string ADDRESS{"127.0.0.1:500"};

namespace {
    std::string readFile(const std::string& path) {
        const std::ifstream f(path);
        std::stringstream ss;
        ss << f.rdbuf();

        return ss.str();
    }
}

namespace server {

    namespace {
        class UserServiceImpl final: public myapp::UserService::Service {
        public:
            grpc::Status GetUser(grpc::ServerContext *context, const myapp::UserRequest *request,
                                 myapp::UserResponse *response) override {
                // AuthContext доступен ТОЛЬКО благодаря TLS/mTLS — здесь можно
                // прочитать, каким сертификатом представился клиент.
                const auto auth_ctx{context->auth_context()};
                std::cout << std::format("[server] GetUser called. is_peer_authenticated= {}\n",
                                         auth_ctx->IsPeerAuthenticated());
                for (const auto& prop: auth_ctx->FindPropertyValues("x509_common_name")) {
                    std::cout << std::format("  client cerCN= {}\n", std::string(prop.data(), prop.length()));
                }

                response->set_user_id(1);
                response->set_name("Alice");
                response->set_email("alice@example.com");

                return grpc::Status::OK;
            }
        };
    }

    static int start(const bool require_client_cert) {
        std::cout
            << "require_client_cert: "
            << std::boolalpha
            << require_client_cert
            << std::noboolalpha << std::endl;
        UserServiceImpl service;

        grpc::SslServerCredentialsOptions ssl_opts{
            require_client_cert
            ? GRPC_SSL_REQUEST_AND_REQUIRE_CLIENT_CERTIFICATE_AND_VERIFY
            : GRPC_SSL_DONT_REQUEST_CLIENT_CERTIFICATE
        };

        grpc::SslServerCredentialsOptions::PemKeyCertPair key_cert_pair;
        key_cert_pair.private_key = readFile("certs/server.key");
        key_cert_pair.cert_chain = readFile("certs/server.crt");
        ssl_opts.pem_key_cert_pairs.push_back(key_cert_pair);

        if (require_client_cert) {
            // pem_root_certs здесь = CA, которым должны быть подписаны сертификаты
            // КЛИЕНТОВ — сервер проверяет ими входящие клиентские сертификаты.
            ssl_opts.pem_root_certs = readFile("certs/ca.crt");
        }

        const auto creds{SslServerCredentials(ssl_opts)};

        grpc::ServerBuilder builder;
        builder.AddListeningPort(ADDRESS, creds);
        builder.RegisterService(&service);

        const std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
        std::cout << std::format("[server] listen to {} {}\n",
            ADDRESS,
            (require_client_cert
                ? "mTLS: need client crt"
                : "TLS: only server crt"));
        server->Wait();

        return 0;
    }

}

namespace client {
    static int start(const bool send_client_cert) {
        std::cout << "send_client_cert: "
            << std::boolalpha
            << send_client_cert
            << std::noboolalpha << std::endl;
        grpc::SslCredentialsOptions ssl_opts;
        // pem_root_certs здесь = CA, которым должен быть подписан сертификат
        // СЕРВЕРА — так клиент проверяет подлинность сервера (это и есть базовый TLS).
        ssl_opts.pem_root_certs = readFile("certs/ca.crt");

        if (send_client_cert) {
            ssl_opts.pem_private_key = readFile("certs/client.key");
            ssl_opts.pem_cert_chain = readFile("certs/client.crt");
        }

        grpc::ChannelArguments args;
        // т.к. в сертификате указан CN/SAN=localhost, а не 127.0.0.1 в общем случае —
        // переопределяем имя, с которым сверяется сертификат сервера
        args.SetSslTargetNameOverride("localhost");

        const auto creds{grpc::SslCredentials(ssl_opts)};
        const auto channel{grpc::CreateCustomChannel(ADDRESS, creds, args)};
        std::unique_ptr<myapp::UserService::Stub> stub{
            myapp::UserService::NewStub(channel)
        };

        myapp::UserRequest request;
        request.set_user_id(1);
        myapp::UserResponse response;
        grpc::ClientContext context;

        std::cout << std::format("[client, {}] calling GetUser(...)\n",
            (send_client_cert ? "crt" : "no crt"));
        const auto status{stub->GetUser(&context, request, &response)};

        const auto ok{status.ok()};
        std::cout << std::format("[client] status.ok()= {}, ", ok);
        if (ok) {
            std::cout << std::format(", name= {}\n", response.name());
        } else {
            std::cout << std::format(", code= {}, error= {}\n",
                static_cast<int>(status.error_code()),
                status.error_message());
        }

        return ok ? 0 : 1;
    }
}



int main(const int argc, char *argv[]) {
    if (argc != 3) {
        std::cout << "BAD ARGS\n";
        return 1;
    }

    const std::string type{argv[1]};
    const std::string param{argv[2]};
    std::cout << std::format("params: '{}', '{}'\n", type, param);

    int code{};
    if (type == "server") {
        code = server::start(param == "mtls");
    } else if (type == "client") {
        code = client::start(param == "with-cert");
    } else {
        std::cout << "BAD TYPE\n";
        code = 1;
    }

    return code;
}

/*

.\build\debug\Debug\demo.exe server mtls
.\build\debug\Debug\demo.exe server -
.\build\debug\Debug\demo.exe client with-cert
.\build\debug\Debug\demo.exe client -

*/