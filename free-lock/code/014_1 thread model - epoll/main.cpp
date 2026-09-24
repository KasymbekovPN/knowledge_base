// ============================================================
// Epoll-style reactor + worker pool, кросс-платформенный (Windows/Linux).
//
// ВАЖНАЯ ОГОВОРКА: настоящего epoll на Windows не существует --
// ближайший readiness-based аналог с похожим API это WSAPoll,
// но он O(n) на каждый wait() (полное сканирование списка fd),
// в отличие от epoll, который O(1) относительно числа ОТСЛЕЖИВАЕМЫХ
// fd (стоимость зависит только от числа ГОТОВЫХ). Для настоящей
// epoll-эквивалентной масштабируемости на Windows нужен IOCP --
// принципиально другая, completion-based модель (см. пояснение
// в конце файла), несовместимая по структуре кода с этим реактором.
// Этот файл даёт одинаковый ИНТЕРФЕЙС и работающий код на обеих
// платформах, но не одинаковую асимптотику при больших N.
//
// Linux:   g++ -std=c++20 -O2 -pthread event_loop_portable.cpp -o server
// Windows: cl /std:c++20 /EHsc event_loop_portable.cpp /link ws2_32.lib
//          (MinGW: g++ -std=c++20 -O2 event_loop_portable.cpp -o server.exe -lws2_32)
// ============================================================

// ============================================================
// Про IOCP (для справки, не реализовано здесь):
//
// Настоящий production-grade эквивалент epoll по масштабируемости
// на Windows -- I/O Completion Ports. Модель принципиально другая:
// не "скажи мне, что готово" (readiness), а "вот тебе буфер, положи
// туда данные и разбуди меня, когда закончишь" (completion) --
// концептуально ближе к io_uring, чем к epoll. WSARecv/WSASend с
// overlapped-структурами, GetQueuedCompletionStatus() вместо
// epoll_wait(). Переписать этот реактор на IOCP означало бы
// изменить саму структуру кода (нет единого "read() внутри пула
// событий", вместо этого completion-callback на каждую операцию),
// а не просто подменить одну функцию другой под #ifdef -- поэтому
// в этом файле выбран WSAPoll как ближайший API-совместимый, а не
// асимптотически эквивалентный вариант.
// ============================================================

#include <atomic>
#include <iostream>
#include <format>
#include <thread>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <string>
#include <chrono>
#include <ranges>
#include <new>

namespace {
    // ------------------------------------------------------------
    // Платформенный слой сокетов (тот же, что в thread-per-connection версии)
    // ------------------------------------------------------------
#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

    using socket_t = SOCKET;
    constexpr socket_t INVALID_SOCK = INVALID_SOCKET;

    void close_socket(socket_t s) { closesocket(s); }
    void set_nonblocking(socket_t s) {
        u_long mode = 1;
        ioctlsocket(s, FIONBIO, &mode);
    }

    struct WinsockInit {
        WinsockInit() {
            WSADATA wsa_data;
            if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
                throw std::runtime_error("WSAStartup failed");
            }
        }
        ~WinsockInit() { WSACleanup(); }
    };

#else
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

    using socket_t = int;
    constexpr socket_t INVALID_SOCK = -1;

    inline void close_socket(socket_t s) { close(s); }
    inline void set_nonblocking(socket_t s) {
        int flags = fcntl(s, F_GETFL, 0);
        fcntl(s, F_SETFL, flags | O_NONBLOCK);
    }

    struct WinsockInit {}; // заглушка на Linux
#endif

    int socket_read(const socket_t s, char* buf, const int len) {
        return recv(s, buf, len, 0);
    }

    int socket_write(const socket_t s, const char* buf, const int len) {
        return send(s, buf, len, 0);
    }

    // ============================================================
    // EventPoller: единый интерфейс поверх epoll (Linux) / WSAPoll (Windows).
    // add()/remove() -- регистрация fd на чтение; wait() -- вернуть
    // список fd, готовых к чтению, с таймаутом.
    // ============================================================
#ifdef _WIN32
    // #include <winsock2.h>
    class EventPoller {
        std::vector<WSAPOLLFD> fds_;

    public:
        void add(socket_t fd) {
            WSAPOLLFD pfd{};
            pfd.fd = fd;
            pfd.events = POLLRDNORM;
            fds_.push_back(pfd);
        }

        void remove(socket_t fd) {
            fds_.erase(
                std::ranges::remove_if(fds_, [fd](const WSAPOLLFD& p) { return p.fd == fd; }).begin(),
                fds_.end());
        }

        // O(n) по числу зарегистрированных fd -- WSAPoll всегда сканирует
        // весь переданный массив, в отличие от epoll_wait
        std::vector<socket_t> wait(int timeout_ms) {
            std::vector<socket_t> ready;
            if (fds_.empty()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms));
                return ready;
            }

            if (const int n{WSAPoll(fds_.data(), static_cast<ULONG>(fds_.size()), timeout_ms)};
                n <= 0) return ready;
            for (auto& pfd: fds_) {
                if (pfd.revents & (POLLRDNORM | POLLERR | POLLHUP)) {
                    ready.push_back(pfd.fd);
                }
            }

            return ready;
        }
    };
#else
    class EventPoller {
        int epoll_fd_;

    public:
        EventPoller() { epoll_fd_ = epoll_create1(0); }
        ~EventPoller() { close(epoll_fd_); }

        void add(socket_t fd) {
            epoll_event ev{};
            ev.events = EPOLLIN;
            ev.data.fd = fd;
            epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev);
        }

        void remove(socket_t fd) {
            epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
        }

        // O(1) относительно ОБЩЕГО числа зарегистрированных fd -- ядро
        // само ведёт список готовых, epoll_wait не сканирует всё подряд
        std::vector<socket_t> wait(int timeout_ms) {
            std::vector<socket_t> ready;
            constexpr int MAX_EVENTS = 64;
            epoll_event events[MAX_EVENTS];
            int n = epoll_wait(epoll_fd_, events, MAX_EVENTS, timeout_ms);
            for (int i = 0; i < n; ++i) ready.push_back(events[i].data.fd);
            return ready;
        }
    };
#endif

    // ============================================================
    // MPMC bus (Vyukov) -- та же реализация, что и во всех предыдущих
    // демках, как shared state между реактором и пулом воркеров
    // ============================================================
    template<typename T>
    class MPMSBus {
        struct Cell {
            std::atomic<size_t> sequence;
            T data;
        };

        alignas(std::hardware_destructive_interference_size)
            std::atomic<size_t> enqueue_pos_;
        alignas(std::hardware_destructive_interference_size)
            std::atomic<size_t> dequeue_pos_;
        Cell* buffer_;
        size_t buffer_mask_;
    public:
        explicit MPMSBus(const size_t capacity):
            buffer_{new Cell[capacity]},
            buffer_mask_{capacity - 1}
        {
            for (size_t i{}; i < capacity; ++i) {
                buffer_[i].sequence.store(i, std::memory_order_relaxed);
            }
            enqueue_pos_.store(0, std::memory_order_relaxed);
            dequeue_pos_.store(0, std::memory_order_relaxed);
        }

        ~MPMSBus() { delete[] buffer_; }

        MPMSBus(const MPMSBus&) = delete;

        bool try_push(T value) {
            Cell* cell;
            size_t pos{enqueue_pos_.load(std::memory_order_relaxed)};
            for (;;) {
                cell = &buffer_[pos & buffer_mask_];
                const size_t seq{cell->sequence.load(std::memory_order_acquire)};

                if (const intptr_t dif{static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos)};
                    dif == 0) {
                    if (enqueue_pos_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) break;
                } else if (dif < 0) {
                    return false;
                } else {
                    pos = enqueue_pos_.load(std::memory_order_relaxed);
                }
            }

            cell->data = std::move(value);
            cell->sequence.store(pos + 1, std::memory_order_release);

            return true;
        }

        bool try_pop(T& result) {
            Cell* cell;
            size_t pos{dequeue_pos_.load(std::memory_order_relaxed)};
            for (;;) {
                cell = &buffer_[pos & buffer_mask_];
                const size_t seq{cell->sequence.load(std::memory_order_acquire)};
                if (const intptr_t dif{static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos + 1)};
                    dif == 0) {
                    if (dequeue_pos_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) break;
                } else if (dif < 0) {
                    return false;
                } else {
                    pos = dequeue_pos_.load(std::memory_order_relaxed);
                }
            }

            result = std::move(cell->data);
            cell->sequence.store(pos + buffer_mask_ + 1, std::memory_order_release);

            return true;
        }
    };

    struct Task {
        socket_t client_fd{};
        std::string data;
    };

    // ============================================================
    // Reactor: единственный поток, крутит EventPoller::wait(), никогда
    // не блокируется дольше таймаута. Логика идентична на обеих
    // платформах -- разница спрятана внутри EventPoller.
    // ============================================================
    class Reactor {
        EventPoller poller_;
        socket_t listen_fd_;
        MPMSBus<Task>& task_queue_;
        std::atomic<bool>& running_;

    public:
        explicit Reactor(const socket_t listen_fd,
                         MPMSBus<Task>& task_queue,
                         std::atomic<bool>& running):
            listen_fd_{listen_fd},
            task_queue_{task_queue},
            running_{running} {

            poller_.add(listen_fd_);
        }

        void run() {
            while (running_.load(std::memory_order_relaxed)) {
                for (const auto ready{poller_.wait(100)};
                    const socket_t fd: ready) {
                    if (fd == listen_fd_) {
                        const socket_t client_fd{accept(listen_fd_, nullptr, nullptr)};
                        if (client_fd == INVALID_SOCK) continue;
                        set_nonblocking(client_fd);
                        poller_.add(client_fd);
                    } else {
                        char buf[4096];
                        const int n_read{socket_read(fd, buf, sizeof(buf))};
                        if (n_read <= 0) {
                            poller_.remove(fd);
                            close_socket(fd);
                            continue;
                        }

                        if (Task task{.client_fd = fd, .data = std::string(buf, n_read)};
                            !task_queue_.try_push(std::move(task))) {
                            std::cerr << std::format("[reactor] task queue full, dropping\n") << std::flush;
                        }
                    }
                }
            }
        }
    };

    void worker_loop(const int worker_id,
                     MPMSBus<Task>& queue,
                     const std::atomic<bool>& running,
                     std::atomic<long long>& processed) {
        Task task;
        while (/*running.load(std::memory_order_relaxed) ||*/ true) {
            if (queue.try_pop(task)) {
                std::string response{std::format("[worker {}] echo: {}", worker_id, task.data)};
                socket_write(task.client_fd, response.data(), static_cast<int>(response.size()));
                processed.fetch_add(1, std::memory_order_relaxed);
            } else {
                if (!running.load(std::memory_order_relaxed)) break;
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            }
        }
    }

}



int main() {
    constexpr int PORT{18891};
    constexpr int NUM_WORKERS{4};
    constexpr int  NUM_TEST_CLIENTS{20};

    WinsockInit winsock_guard;

    socket_t listen_fd{socket(AF_INET, SOCK_STREAM, 0)};
    constexpr int opt {1};
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));
    set_nonblocking(listen_fd);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    bind(listen_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    listen(listen_fd, 128);

    MPMSBus<Task> task_queue{1024};
    std::atomic<bool> running{true};
    std::atomic<long long> processed{0};

    Reactor reactor{listen_fd, task_queue, running};
    std::thread reactor_thread{[&reactor] { reactor.run(); }};

    std::vector<std::thread> workers;
    for (int w{}; w < NUM_WORKERS; ++w) {
        workers.emplace_back(
            worker_loop,
            w,
            std::ref(task_queue),
            std::ref(running),
            std::ref(processed));
    }

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

            std::string msg{std::format("hello from client {}", c)};
            socket_write(sock, msg.data(), static_cast<int>(msg.size()));

            char buf[256] = {};
            if (const int n{socket_read(sock, buf, sizeof(buf) - 1)};
                n > 0) {
                const std::string resp{buf};
                std::cout << std::format("{}\n", buf) << std::flush;
                success_count.fetch_add(1, std::memory_order_relaxed);
            }
            close_socket(sock);
        });
    }
    for (auto& t: test_clients) t.join();

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    running.store(false, std::memory_order_relaxed);

    reactor_thread.join();
    for (auto& t: workers) t.join();
    close_socket(listen_fd);

    std::cout << std::format("Success client: {} / {}\n", success_count.load(), NUM_TEST_CLIENTS);
    std::cout << std::format("Total processed tasks: {}\n", processed.load());

    return 0;
}
