// ============================================================
// IOCP echo-сервер (Windows only) -- completion-based аналог
// io_uring_server.cpp. Концептуально та же модель ("submit async
// op -> получить completion позже"), но другой API и другая
// внутренняя механика (нет разделяемых SQ/CQ колец в userspace --
// вместо этого GetQueuedCompletionStatus() вытягивает готовые
// результаты из очереди, которую ведёт ядро).
//
// Сборка (MSVC): cl /std:c++20 /EHsc iocp_server.cpp /link ws2_32.lib
// Сборка (MinGW): g++ -std=c++20 -O2 iocp_server.cpp -o server.exe -lws2_32 -lmswsock
// ============================================================

#ifndef _WIN32
#error "This file must be compiled on Windows only"
#endif

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")

#include <atomic>
#include <iostream>
#include <format>
#include <thread>
#include <vector>
#include <string>
#include <string_view>

namespace {
    enum class OpType { Accept, Read, Write };

    // OVERLAPPED должен быть ПЕРВЫМ полем -- IOCP возвращает указатель
    // именно на OVERLAPPED из GetQueuedCompletionStatus, и мы приводим
    // его обратно к IoContext через reinterpret_cast, полагаясь на то,
    // что адрес OVERLAPPED == адрес IoContext (стандартный C-style
    // "intrusive" приём, аналог того, как мы делали intrusive next-
    // указатели в lock-free очередях)
    struct IoContext {
        OVERLAPPED overlapped{};
        OpType type;
        SOCKET client_fd{INVALID_SOCKET};
        std::vector<char> buffer;
        WSABUF wsabuf{};
    };

    std::atomic<long long> echoed_count{0};
    std::atomic<bool> running{true};

    LPFN_ACCEPTEX AcceptExPtr{nullptr};

    void load_acceptex(const SOCKET listen_fd) {
        GUID guid = WSAID_ACCEPTEX;
        DWORD bytes{0};
        WSAIoctl(
            listen_fd,
            SIO_GET_EXTENSION_FUNCTION_POINTER,
            &guid,
            sizeof(guid),
            &AcceptExPtr,
            sizeof(AcceptExPtr),
            &bytes,
            nullptr,
            nullptr);
    }

    void submit_accept(HANDLE iocp, const SOCKET listen_fd) {
        const SOCKET accept_socket{WSASocket(
            AF_INET,
            SOCK_STREAM,
            IPPROTO_TCP,
            nullptr,
            0,
            WSA_FLAG_OVERLAPPED)};

        auto* ctx{new IoContext()};
        ctx->type = OpType::Accept;
        ctx->client_fd = accept_socket;
        ctx->buffer.resize(64); // под адреса, требуемые AcceptEx

        DWORD bytes_received{0};
        // AcceptEx -- асинхронный аналог accept(): принимает соединение
        // БЕЗ блокировки, результат придёт как IOCP completion, точно
        // так же, как io_uring_prep_accept + последующий CQE
        const BOOL ok{AcceptExPtr(
            listen_fd,
            accept_socket,
            ctx->buffer.data(),
            0,
            sizeof(sockaddr_in) + 16,
            sizeof(sockaddr_in) + 16,
            &bytes_received,
            &ctx->overlapped)};

        if (!ok && WSAGetLastError() != ERROR_IO_PENDING) {
            std::cerr << std::format("AcceptEx failed: {}\n", WSAGetLastError());
            delete ctx;
            closesocket(accept_socket);
        }
        // Если ok==FALSE и ERROR_IO_PENDING -- это НЕ ошибка, это нормальное
        // "операция поставлена в очередь, результат придёт через IOCP" --
        // ровно та же семантика, что "SQE отправлен, ждём CQE" в io_uring
    }

    void submit_read(HANDLE iocp, const SOCKET client_fd) {
        auto* ctx{new IoContext()};
        ctx->type = OpType::Read;
        ctx->client_fd = client_fd;
        ctx->buffer.resize(4096);
        ctx->wsabuf.buf = ctx->buffer.data();
        ctx->wsabuf.len = static_cast<ULONG>(ctx->buffer.size());

        DWORD flags{0};
        DWORD bytes{0};
        if (const int ret{WSARecv(client_fd, &ctx->wsabuf, 1, &bytes, &flags, &ctx->overlapped, nullptr)};
            ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
            delete ctx;
            closesocket(client_fd);
        }
    }

    void submit_write(HANDLE iocp, const SOCKET client_fd, std::string data) {
        auto* ctx{new IoContext()};
        ctx->type = OpType::Write;
        ctx->client_fd = client_fd;
        ctx->buffer.assign(data.begin(), data.end());
        ctx->wsabuf.buf = ctx->buffer.data();
        ctx->wsabuf.len = static_cast<ULONG>(ctx->buffer.size());

        DWORD bytes{0};
        if (const int ret{WSASend(client_fd, &ctx->wsabuf, 1, &bytes, 0, &ctx->overlapped, nullptr)};
            ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
            delete ctx;
            closesocket(client_fd);
        }
    }

    void iocp_loop(HANDLE iocp, SOCKET listen_fd) {
        while (running.load(std::memory_order_relaxed)) {
            DWORD bytes_transferred{0};
            ULONG_PTR completion_key{0};
            OVERLAPPED* overlapped{nullptr};

            // GetQueuedCompletionStatus -- прямой аналог io_uring_wait_cqe_timeout:
            // блокируется до готового результата ИЛИ таймаута (100мс, чтобы
            // периодически проверять running -- тот же приём, что и в io_uring
            // версии, и в epoll-версии до этого)
            const BOOL ok{GetQueuedCompletionStatus(
                iocp,
                &bytes_transferred,
                &completion_key,
                &overlapped,
                100)};

            // таймаут -- проверяем running и продолжаем
            if (overlapped == nullptr) continue;

            // Тот самый intrusive-трюк: overlapped -- это первое поле IoContext,
            // поэтому адрес overlapped == адрес IoContext
            auto* ctx{reinterpret_cast<IoContext*>(overlapped)};

            if (ctx->type == OpType::Accept) {
                if (!ok) {
                    // AcceptEx не завершился успешно -- закрываем неудавшийся
                    // accept_socket и переотправляем accept, иначе сервер
                    // перестанет принимать новые соединения
                    closesocket(ctx->client_fd);
                    delete ctx;
                    submit_accept(iocp, listen_fd);
                    continue;
                }
                // ok == TRUE -- успешный accept, bytes_transferred==0 здесь
                // ожидаемо (мы просили 0 байт initial data), это НЕ ошибка
            } else if (!ok || bytes_transferred == 0) {
                // Read/Write: !ok -- ошибка операции; bytes_transferred==0 при
                // ok==TRUE -- graceful disconnect для стрим-сокета
                closesocket(ctx->client_fd);
                delete ctx;
                continue;
            }

            switch (ctx->type) {
                case OpType::Accept: {
                    CreateIoCompletionPort(
                        reinterpret_cast<HANDLE>(ctx->client_fd),
                        iocp,
                        0,
                        0);
                    submit_read(iocp, ctx->client_fd);
                    // сразу снова слушаем следующее подключение
                    submit_accept(iocp, listen_fd);
                    delete ctx;

                    break;
                }
                case OpType::Read: {
                    std::string request{ctx->buffer.data(), bytes_transferred};
                    std::string response{std::format("[iocp] echo: {}", request)};
                    std::cout << std::format("RESP: {}\n", response);
                    SOCKET fd{ctx->client_fd};
                    delete ctx;
                    submit_write(iocp, fd, std::move(response));
                    break;
                }
                case OpType::Write: {
                    echoed_count.fetch_add(1, std::memory_order_relaxed);
                    SOCKET fd{ctx->client_fd};
                    delete ctx;
                    // ждём следующее сообщение от этого клиента
                    submit_read(iocp, fd);

                    break;
                }
            }
        }
    }

}

int main() {
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2, 2), &wsa_data);

    constexpr int PORT{18893};

    SOCKET listen_fd{WSASocket(
        AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP,
        nullptr,
        0,
        WSA_FLAG_OVERLAPPED)};

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(listen_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        std::cerr << std::format("bind() failed: {}\n", WSAGetLastError());
        return 1;
    }
    listen(listen_fd, 128);

    load_acceptex(listen_fd);

    // IOCP handle -- аналог io_uring_queue_init(): создаём "порт
    // завершения", к которому потом привязываем listen_fd и каждый
    // новый client_fd через CreateIoCompletionPort
    HANDLE iocp{CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0)};
    CreateIoCompletionPort(reinterpret_cast<HANDLE>(listen_fd), iocp, 0, 0);

    submit_accept(iocp, listen_fd);
    std::thread iocp_thread{iocp_loop, iocp, listen_fd};

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    constexpr int NUM_TEST_CLIENTS{20};
    std::vector<std::thread> test_clients;
    std::atomic<int> success_count{0};
    for (int c{}; c < NUM_TEST_CLIENTS; ++c) {
        test_clients.emplace_back([&, c] {
            SOCKET sock{socket(AF_INET, SOCK_STREAM, 0)};
            sockaddr_in caddr{};
            caddr.sin_family = AF_INET;
            caddr.sin_port = htons(PORT);
            inet_pton(AF_INET, "127.0.0.1", &caddr.sin_addr);

            if (connect(sock, reinterpret_cast<sockaddr*>(&caddr), sizeof(caddr)) != 0) {
                closesocket(sock);
                return;
            }

            std::string msg{std::format("hello from client {}", c)};
            send(sock, msg.data(), static_cast<int>(msg.size()), 0);

            char buf[256] = {};
            if (const int n{recv(sock, buf, sizeof(buf) - 1, 0)}; n > 0) {
                std::cout << std::format("{}\n", std::string_view{buf, static_cast<size_t>(n)}) << std::flush;
                success_count.fetch_add(1, std::memory_order_relaxed);
            }
            closesocket(sock);
        });
    }
    for (auto& t: test_clients) t.join();

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    running.store(false, std::memory_order_relaxed);
    iocp_thread.join();

    std::cout << std::format("Success clients: {} / {}\n", success_count.load(), NUM_TEST_CLIENTS);
    std::cout << std::format("Echo processed: {}\n", echoed_count.load());

    closesocket(listen_fd);
    CloseHandle(iocp);
    WSACleanup();

    return 0;
}
