// ============================================================
// Thread-per-connection сервер, кросс-платформенный (Windows/Linux).
//
// Linux:   g++ -std=c++20 -O2 -pthread thread_per_connection_portable.cpp -o server
// Windows: cl /std:c++20 /EHsc thread_per_connection_portable.cpp /link ws2_32.lib
//          (или MinGW: g++ -std=c++20 -O2 thread_per_connection_portable.cpp -o server.exe -lws2_32)
// ============================================================

#include <atomic>
#include <iostream>
#include <format>
#include <thread>
#include <vector>
#include <string>
#include <mutex>
#include <chrono>
#include <cstring>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")

    using socket_t = SOCKET;
    constexpr socket_t INVALID_SOCK = INVALID_SOCKET;

    inline void close_socket(socket_t s) { closesocket(s); }
    inline int last_error() { return WSAGetLastError(); }

    // WSAPoll появился в Vista+ и имеет тот же интерфейс, что poll()
    // на POSIX -- structs совместимы по именам полей (fd, events, revents)
    inline int poll_fds(WSAPOLLFD* fds, unsigned long count, int timeout_ms) {
        return WSAPoll(fds, count, timeout_ms);
    }
    using pollfd_t = WSAPOLLFD;

    struct WinsockInit {
        WinsockInit() {
            WSADATA wsa_data;
            if (WSAStartup(MAKEWORD(2, 2 ), &wsa_data) != 0) {
                throw std::runtime_error("WSAStartup failed");
            }
        }
        ~WinsockInit() { WSACleanup(); }
};

#else
     #include <sys/socket.h>
     #include <netinet/in.h>
     #include <netinet/tcp.h>
     #include <arpa/inet.h>
     #include <unistd.h>
     #include <poll.h>
     #include <errno.h>

     using socket_t = int;
     constexpr socket_t INVALID_SOCK = -1;

     inline void close_socket(socket_t s) { close(s); }
     inline int  last_error() { return errno; }

     inline int poll_fds(pollfd* fds, nfds_t count, int timeout_ms) {
         return poll(fds, count, timeout_ms);
     }
     using pollfd_t = pollfd;

     struct WinsockInit {}; // на Linux инициализация не нужна -- пустая заглушка
#endif

namespace {
    // recv()/send() -- НЕ read()/write() -- работают одинаково на обеих
    // платформах для сокетов (единственный по-настоящему кросс-платформенный
    // способ читать/писать в socket без #ifdef на каждый вызов; на Windows
    // сокет -- не файловый дескриптор в POSIX-смысле, read()/write() для
    // него в общем случае недоступны)
    int socket_read(socket_t s, char* buf, int len) {
        return recv(s, buf, len, 0);
    }

    int socket_write(socket_t s, const char* buf, int len) {
        return send(s, buf, len, 0);
    }

    std::atomic<int> online_clients{0};

    std::mutex log_mutex;
    void log_line(const std::string& msg) {
        std::lock_guard<std::mutex> log(log_mutex);
        std::cout << std::format("{}\n", msg) << std::flush;
    }

    void handle_connection(const socket_t client_fd, int connection_id) {
        online_clients.fetch_add(1, std::memory_order_relaxed);
        log_line(std::format("[conn {} on, online= {}]",
            connection_id,
            online_clients.load()));

        char buf[4096];
        while (true) {
            const int n{socket_read(client_fd, buf, sizeof(buf))};
            if (n <= 0) break;

            std::string request(buf, n);
            std::string response{std::format("[conn {}, echo: '{}']", connection_id, request)};
            std::cout << std::format("RESP: {}\n", response) << std::flush;
            socket_write(client_fd, response.data(), static_cast<int>(response.size()));
        }

        close_socket(client_fd);
        online_clients.fetch_add(1, std::memory_order_relaxed);
        log_line(std::format("[conn {} off, online= {}]", connection_id, online_clients.load()));
    }

    // close() слушающего сокета
    // из другого потока не гарантированно прерывает блокирующий accept()
    // ни на Linux, ни тем более на Windows -- используем poll с таймаутом
    // одинаково на обеих платформах через platform-слой выше.
    void accept_loop(const socket_t listen_fd, const std::atomic<bool>& running, std::atomic<int>& connection_counter) {
        while (running.load(std::memory_order_relaxed)) {
            pollfd_t pfd{};
            pfd.fd = listen_fd;
            pfd.events = POLLIN;

            if (const int ready{poll_fds(&pfd, 1, 100)};
                ready <= 0) continue;

            const socket_t client_fd{accept(listen_fd, nullptr, nullptr)};
            if (client_fd == INVALID_SOCK) continue;

            int id{connection_counter.fetch_add(1, std::memory_order_relaxed)};
            std::thread{handle_connection, client_fd, id}.detach();
        }
    }

}

int main() {
    constexpr int PORT{18890};
    constexpr int NUM_TEST_CLIENTS{30};

    // RAII: на Windows поднимает Winsock при входе в scope, гасит при
    // выходе. На Linux -- пустая структура, ничего не делает.
    WinsockInit winsock_guard;

    const socket_t listen_fd{socket(AF_INET, SOCK_STREAM, 0)};
    if (listen_fd == INVALID_SOCK) {
        std::cerr << std::format("socket() failed: {}\n", last_error());
        return 1;
    }

    constexpr int opt{1};
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(listen_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        std::cerr << std::format("bind() failed: {}\n", last_error());
        return 1;
    }
    listen(listen_fd, 128);

    std::atomic<bool> running{true};
    std::atomic<int> connection_counter{0};

    std::thread acceptor{accept_loop, listen_fd, std::ref(running), std::ref(connection_counter)};
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::vector<std::thread> test_clients;
    std::atomic<int> success_count{0};
    for (int c{}; c < NUM_TEST_CLIENTS; ++c) {
        test_clients.emplace_back([&, c] {
            socket_t sock{socket(AF_INET, SOCK_STREAM, 0)};
            sockaddr_in caddr{};
            caddr.sin_family = AF_INET;
            caddr.sin_port = htons(PORT);
            inet_pton(AF_INET, "127.0.0.1", &caddr.sin_addr);

            if (connect(sock, reinterpret_cast<sockaddr*>(&caddr), sizeof(caddr)) != 0) {
                close_socket(sock);
                return;
            }

            bool all_ok{true};
            for (int msg_i{}; msg_i < 3; ++msg_i) {
                std::string msg{std::format("client {} msg {}", c, msg_i)};
                if (const int w{socket_write(sock, msg.data(), static_cast<int>(msg.size())) };
                    w <= 0) {
                    all_ok = false;
                    break;
                }

                char buf[256] = {};
                if (const int n{socket_read(sock, buf, sizeof(buf) - 1)};
                    n <= 0) {
                    all_ok = false;
                    break;
                }
                // std::cout << sizeof(buf) << " !!! " << buf << std::endl;
                const std::string result{buf};
                // std::cout << result.size() << " +++ " << result << std::endl;
            }

            close_socket(sock);
            if (all_ok) success_count.fetch_add(1, std::memory_order_relaxed);
        });
    }

    for (auto& t: test_clients) t.join();

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    running.store(false, std::memory_order_relaxed);
    acceptor.join();
    close_socket(listen_fd);

    std::cout << std::format("\nSuccessful client: {} / {}\n", success_count.load(), NUM_TEST_CLIENTS);
    std::cout << std::format("Total online {} (must be zero)\n", online_clients.load());

    return 0;
}
