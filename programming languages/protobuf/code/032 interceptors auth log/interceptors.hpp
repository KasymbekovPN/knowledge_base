#pragma once

#include <chrono>
#include <iostream>
#include <format>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include <grpcpp/support/client_interceptor.h>
#include <grpcpp/support/server_interceptor.h>

// ===================== КЛИЕНТСКИЙ ИНТЕРЦЕПТОР =====================
// Добавляет auth-токен в исходящие метаданные КАЖДОГО вызова и логирует
// имя метода + время выполнения — без единой строчки этого кода в
// самих местах вызова stub->Method(...).
class ClientAuthLoggingInterceptor: public grpc::experimental::Interceptor {
public:
    explicit ClientAuthLoggingInterceptor(grpc::experimental::ClientRpcInfo* info):
        info_{info} {}

    // void Intercept(grpc::experimental::Interceptor) {}
    //   void Intercept(grpc::experimental::InterceptorBatchMethods* methods) override {
    //     if (methods->QueryInterceptionHookPoint(
    //             grpc::experimental::InterceptionHookPoints::PRE_SEND_INITIAL_METADATA)) {
    //       start_ = std::chrono::steady_clock::now();
    //       auto* metadata = methods->GetSendInitialMetadata();
    //       metadata->insert({"authorization", "Bearer secret-token-123"});
    //       std::cout << "[client-interceptor] -> " << info_->method()
    //                  << ": добавлен auth-токен\n";
    //     }
    //
    //     if (methods->QueryInterceptionHookPoint(
    //             grpc::experimental::InterceptionHookPoints::POST_RECV_STATUS)) {
    //       auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
    //           std::chrono::steady_clock::now() - start_).count();
    //       grpc::Status* status = methods->GetRecvStatus();
    //       std::cout << "[client-interceptor] <- " << info_->method()
    //                  << ": завершён за " << elapsed << " мс, ok="
    //                  << status->ok() << "\n";
    //     }
    //
    //     methods->Proceed();  // обязательно — иначе RPC зависнет
    //   }
private:
    grpc::experimental::ClientRpcInfo* info_{nullptr};
    std::chrono::steady_clock::time_point start_;
};

// class ClientAuthLoggingInterceptorFactory
//     : public grpc::experimental::ClientInterceptorFactoryInterface {
//  public:
//   grpc::experimental::Interceptor* CreateClientInterceptor(
//       grpc::experimental::ClientRpcInfo* info) override {
//     return new ClientAuthLoggingInterceptor(info);
//   }
// };
//
// // ===================== СЕРВЕРНЫЙ ИНТЕРЦЕПТОР =====================
// // Логирует каждый входящий вызов и проверяет auth-токен. Если токена нет
// // или он неверный — подменяет исходящий статус на UNAUTHENTICATED.
// class ServerAuthLoggingInterceptor : public grpc::experimental::Interceptor {
//  public:
//   explicit ServerAuthLoggingInterceptor(grpc::experimental::ServerRpcInfo* info)
//       : info_(info) {}
//
//   void Intercept(grpc::experimental::InterceptorBatchMethods* methods) override {
//     if (methods->QueryInterceptionHookPoint(
//             grpc::experimental::InterceptionHookPoints::POST_RECV_INITIAL_METADATA)) {
//       auto* metadata = methods->GetRecvInitialMetadata();
//       auto it = metadata->find("authorization");
//       if (it != metadata->end()) {
//         token_ = std::string(it->second.data(), it->second.length());
//       }
//       std::cout << "[server-interceptor] <- " << info_->method()
//                  << ": получен токен = \""
//                  << (token_.empty() ? "(пусто)" : token_) << "\"\n";
//     }
//
//     if (methods->QueryInterceptionHookPoint(
//             grpc::experimental::InterceptionHookPoints::PRE_SEND_STATUS)) {
//       if (token_ != "Bearer secret-token-123") {
//         std::cout << "[server-interceptor] -> " << info_->method()
//                    << ": токен неверный/отсутствует, подменяю статус на UNAUTHENTICATED\n";
//         methods->ModifySendStatus(
//             grpc::Status(grpc::StatusCode::UNAUTHENTICATED, "invalid or missing token"));
//       } else {
//         std::cout << "[server-interceptor] -> " << info_->method()
//                    << ": токен валиден, статус не трогаем\n";
//       }
//     }
//
//     methods->Proceed();
//   }
//
//  private:
//   grpc::experimental::ServerRpcInfo* info_;
//   std::string token_;
// };
//
// class ServerAuthLoggingInterceptorFactory
//     : public grpc::experimental::ServerInterceptorFactoryInterface {
//  public:
//   grpc::experimental::Interceptor* CreateServerInterceptor(
//       grpc::experimental::ServerRpcInfo* info) override {
//     return new ServerAuthLoggingInterceptor(info);
//   }
// };