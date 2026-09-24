


### **Модель потоков**: thread-per-connection vs event loop (epoll/io_uring) + пул воркеров — lock-free структуры чаще всего нужны именно во второй модели, как shared-state между воркерами

# Thread-per-connection vs Event Loop + Worker Pool

## Модель 1: Thread-per-connection

```cmake
cmake_minimum_required(VERSION 4.4.2)
project(demo CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(demo main.cpp)

if (WIN32)
    target_link_libraries(demo PRIVATE ws2_32)
endif()

```

```cpp
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

```


```
Клиент 1 → Поток 1 (блокирующий read/write)
Клиент 2 → Поток 2 (блокирующий read/write)
...
Клиент N → Поток N
```

Каждое соединение — свой ОС-поток, который блокируется на `read()`/`write()`. Просто в коде (обычный последовательный стиль на поток), но:

- **Не масштабируется** — тысячи соединений = тысячи потоков, каждый со своим стеком (обычно 1-8 МБ) и накладными расходами планировщика ОС на context switch.
- **Shared state между потоками нужен редко** — если у каждого клиента своя изолированная логика (например, простой request-response без взаимодействия между клиентами), потокам почти нечего делить. Там, где всё же нужно (общий счётчик онлайн-пользователей, broadcast), достаточно обычного `std::mutex` — редких обращений не хватит, чтобы contention на локе стал узким местом.

**Именно поэтому lock-free здесь обычно избыточен**: конкуренция за shared state пропорциональна частоте обращений к нему, а не количеству потоков как таковому — при тысячах простаивающих на blocking I/O потоков реальный contention на разделяемых структурах может быть низким.

### Model 2: epoll

```cmake
cmake_minimum_required(VERSION 4.4.2)
project(demo CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(demo main.cpp)

if (WIN32)
    target_link_libraries(demo PRIVATE ws2_32)
endif()

```

```cpp
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

```

```
                    ┌─────────────┐
Клиенты 1..N  →   │ epoll/io_uring │  (1 поток, ставит fd на epoll_wait)
                    └──────┬──────┘
                            │ push() -- lock-free
                            ▼
                 ┌──────────────────┐
                 │  MPMC task queue     │  <-- SHARED STATE
                 └─────────┬────────┘
                    ┌───────┼───────┐
                    ▼       ▼       ▼
                Worker 1  Worker 2  Worker N   (pop(), обрабатывают)
```

Один (или несколько) поток-реактор мультиплексирует **все** соединения через `epoll`/`io_uring` — не блокируется на конкретном клиенте, а ждёт "готовности" сразу у многих fd. Реальная обработка (бизнес-логика) уходит в пул воркеров.

**Здесь lock-free почти обязателен**, и вот почему — это ключевая часть ответа на ваш вопрос:

Реактор — это **единственная точка**, через которую проходят события со всех соединений разом. Если передача задачи от реактора воркеру идёт через `std::mutex`, и воркер (или другой поток) в этот момент держит лок дольше обычного — реактор блокируется на `mutex::lock()`, а значит **не может вернуться к `epoll_wait()`**, а значит **все остальные клиенты замирают**, даже если их данные уже готовы и просто ждут, пока реактор освободится. Один медленный контеншн — и деградация касается всех соединений одновременно, а не одного.

С lock-free `push()` (как в MPMC выше) реактор в худшем случае прокрутит несколько итераций CAS-цикла — микросекунды, не блокировка на неопределённое время из-за чужого потока.

## Разбор ключевого места в коде

```cpp
// Реактор -- единственный поток, крутящий epoll_wait
ssize_t n_read = read(fd, buf, sizeof(buf));
Task task{fd, std::string(buf, n_read)};
if (!task_queue_.push(std::move(task))) {
    // backpressure -- НЕ блокируемся, а сигнализируем и продолжаем
    std::cerr << "queue full, dropping\n";
}
// сразу возвращаемся в epoll_wait -- ни один клиент не ждёт из-за другого
```

```cpp
// N воркеров -- КОНКУРЕНТНО читают из ОДНОЙ и той же очереди
void worker_loop(...) {
    while (...) {
        if (queue.pop(task)) { /* обработка */ }
    }
}
```

Именно **множественность consumer'ов** над **общей** очередью — то, ради чего здесь нужен MPMC, а не MPSC (с MPSC пришлось бы городить отдельную очередь на каждого воркера и балансировку между ними вручную).

## Что стоит доработать для реального прода

- **Backpressure сейчас — просто drop с логом.** В реальности стоит либо динамически растить `capacity`, либо явно отвечать клиенту "перегружен, попробуйте позже", либо (для чата) применять policy "drop oldest" для менее критичных сообщений (presence-обновления) и не дропать критичные (сами сообщения чата).
- **`write()` внутри воркера — потенциальная гонка**, если два воркера одновременно обрабатывают задачи для одного и того же `fd` (в демо это не проявляется, т.к. каждый клиент шлёт одно сообщение, но в реальном чате один и тот же коннект может получить несколько сообщений почти одновременно от разных воркеров). Это ровно то место, где нужна **per-client MPSC-очередь на запись** (Блок 3, следующий пункт) — вместо прямого `write()` из воркера, воркер кладёт исходящее сообщение в MPSC-очередь конкретного клиента, а отдельный writer-поток (или сам реактор через `EPOLLOUT`) её вычитывает и пишет в сокет **строго последовательно**.
- **`io_uring`** решил бы то же самое, что делает epoll здесь, но асинхронно и без syscall на каждый `read`/`write` — концептуально роль lock-free очереди между реактором и воркерами не меняется, меняется только механизм получения событий готовности.

### Model 3: io_uring / iocp

**`iocp_server.cpp` (Windows)** 

```cpp
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

```


**`io_uring_server.cpp` (Linux)** 

```cpp
// ============================================================
// io_uring echo-сервер (Linux only) -- прямая демонстрация
// completion-based модели из теоретического разбора: submit
// SQE ("прочитай в этот буфер"), получить CQE ("готово, вот
// сколько байт"), без цикла readiness-опроса вообще.
//
// sudo apt install -y liburing-dev
// Сборка: g++ -std=c++20 -O2 -pthread io_uring_server.cpp -o server -luring
// ============================================================

#include <liburing.h>
#include <iostream>
#include <format>
#include <atomic>
#include <thread>
#include <vector>
#include <unordered_map>
#include <cstring>
#include <cerrno>
#include <chrono>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace {
    // Каждой in-flight операции нужен "билет" -- что это была за
    // операция и над каким fd, чтобы правильно обработать CQE
    enum class OpType { Accept, Read, Write };

    struct Operation {
        OpType type;
        int fd;
        std::vector<char> buffer;
    };

    class IoUringServer {
        io_uring ring_;
        int listen_fd_;
        std::atomic<bool>& running_;
        std::atomic<long long>& echoed_count_;

        // io_uring identifies completions via user_data (a 64-bit opaque
        // value attached to each SQE). Храним реальные Operation через
        // указатель, упакованный в user_data -- ЖИЗНЕННЫЙ ЦИКЛ этого
        // Operation должен пережить submit и дожить до completion --
        // концептуально ТА ЖЕ reclamation problem, что разбирали для
        // hazard pointers: нельзя освободить память, пока ядро всё ещё
        // "держит" на неё ссылку через in-flight операцию.
        std::unordered_map<Operation*, std::unique_ptr<Operation>> in_flight_;

        void submit_accept() {
            auto* op{new Operation{.type = OpType::Accept, .fd = listen_fd_, .buffer = {}}};
            in_flight_[op] = std::unique_ptr<Operation>(op);

            io_uring_sqe* sqe{io_uring_get_sqe(&ring_)};
            io_uring_prep_accept(sqe, listen_fd_, nullptr, nullptr, 0);
            io_uring_sqe_set_data(sqe, op);
        }

        void submit_read(const int fd) {
            auto* op{new Operation{.type = OpType::Read, .fd = fd, .buffer = std::vector<char>(4096)}};
            in_flight_[op] = std::unique_ptr<Operation>(op);

            io_uring_sqe* sqe{io_uring_get_sqe(&ring_)};
            io_uring_prep_read(sqe, fd, op->buffer.data(), op->buffer.size(), 0);
            io_uring_sqe_set_data(sqe, op);
        }

        void submit_write(const int fd, std::string data) {
            auto* op{new Operation{.type = OpType::Write, .fd = fd, .buffer = {}}};
            op->buffer.assign(data.begin(), data.end());
            in_flight_[op] = std::unique_ptr<Operation>(op);

            io_uring_sqe* sqe{io_uring_get_sqe(&ring_)};
            io_uring_prep_write(sqe, fd, op->buffer.data(), op->buffer.size(), 0);
            io_uring_sqe_set_data(sqe, op);
        }

    public:
        explicit IoUringServer(const int listen_fd, std::atomic<bool>& running, std::atomic<long long>& echoed_count)
            : listen_fd_{listen_fd}, running_{running}, echoed_count_ {echoed_count} {
            io_uring_queue_init(256, &ring_, 0);
        }
        ~IoUringServer() { io_uring_queue_exit(&ring_); }

        void run() {
            submit_accept();
            io_uring_submit(&ring_);

            while (running_.load(std::memory_order_relaxed)) {
                io_uring_cqe* cqe;
                // 100мс -- проверять running периодически
                __kernel_timespec timeout{.tv_sec = 0, .tv_nsec = 100'000'000};
                const int ret{io_uring_wait_cqe_timeout(&ring_, &cqe, &timeout)};
                if (ret == -ETIME) continue; // таймаут -- просто снова проверяем running
                if (ret < 0) continue;

                auto* op{static_cast<Operation*>(io_uring_cqe_get_data(cqe))};
                const int result{cqe->res};
                io_uring_cqe_seen(&ring_, cqe);

                switch (op->type) {
                    case OpType::Accept: {
                        if (result >= 0) {
                            const int client_fd{result}; // semantic clarity
                            submit_read(client_fd); // сразу заказываем чтение с нового клиента
                        }
                        // и сразу снова слушаем следующее подключение
                        submit_accept();
                        break;
                    }
                    case OpType::Read: {
                        if (result <= 0) {
                            // клиент отключился или ошибка
                            close(op->fd);
                        } else {
                            std::string request(op->buffer.data(), result);
                            std::string response{std::format("[io_uring] echo: {}", request)};
                            submit_write(op->fd, std::move(response));
                        }
                        break;
                    }
                    case OpType::Write: {
                        if (result > 0) {
                            echoed_count_.fetch_add(1, std::memory_order_relaxed);
                            // после ответа -- снова ждём следующее сообщение
                            submit_read(op->fd);
                        } else {
                            close(op->fd);
                        }
                        break;
                    }
                }

                // Завершённая операция -- освобождаем Operation. Это безопасно
                // ИМЕННО ЗДЕСЬ, потому что CQE уже получен -- ядро гарантированно
                // больше не обращается к этому буферу (аналог: hazard снят,
                // можно retire). Если бы мы освободили буфер ДО получения CQE --
                // это был бы use-after-free на стороне ядра, симметрично тому,
                // что разбирали для Treiber stack.
                in_flight_.erase(op);
                io_uring_submit(&ring_);
            }
        }
    };

}

int main(int argc, char *argv[]) {
    constexpr int PORT{18892};
    constexpr int NUM_TEST_CLIENTS{20};

    const int listen_fd{socket(AF_INET, SOCK_STREAM, 0)};
    if (listen_fd < 0) {
        std::cerr << std::format("socket() failed: {}\n", strerror(errno));
        return 1;
    }
    const int opt{1};
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    if (bind(listen_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        // ВАЖНО: если запускаете этот сервер повторно на том же порту
        // сразу друг за другом (как я делал при отладке), возможна
        // "Address already in use" -- SO_REUSEADDR не всегда мгновенно
        // освобождает порт. Явная проверка здесь -- не паранойя, а то,
        // из-за отсутствия чего я сам словил "фантомную" нестабильность
        // при тестировании: без проверки код тихо продолжал бы работать
        // все отведённые секунды, ничего не принимая, и выглядело бы
        // как случайная гонка в io_uring, а не как обычная ошибка bind().
        std::cerr << std::format("bind() failed: {}\n", strerror(errno));
        return 1;
    }

    if (listen(listen_fd, 128) < 0) {
        std::cerr << std::format("listen() failed: {}\n", strerror(errno));
        return 1;
    }

    std::atomic<bool> running{true};
    std::atomic<long long> echoed_count{0};

    IoUringServer server{listen_fd, running, echoed_count};
    std::thread server_thread{[&server] { server.run(); }};

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::vector<std::thread> test_clients;
    std::atomic<int> success_count{0};
    for (int c{}; c < NUM_TEST_CLIENTS; ++c) {
        test_clients.emplace_back([&, c] {
            const int sock{socket(AF_INET, SOCK_STREAM, 0)};
            sockaddr_in caddr{};
            caddr.sin_family = AF_INET;
            caddr.sin_port = htons(PORT);
            inet_pton(AF_INET, "127.0.0.1", &caddr.sin_addr);

            if (connect(sock, reinterpret_cast<sockaddr*>(&caddr), sizeof(caddr)) < 0) {
                close(sock);
                return;
            }

            const std::string msg{std::format("hello from client {}", c)};
            write(sock, msg.data(), msg.size());

            char buf[256] = {};
            if (const ssize_t n{read(sock, buf, sizeof(buf) - 1)};
                n > 0) {
                const std::string result{buf};
                std::cout << std::format("result: {}\n", result) << std::flush;
                success_count.fetch_add(1, std::memory_order_relaxed);
            }
            close(sock);
        });
    }

    for (auto& t: test_clients) t.join();

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    running.store(false, std::memory_order_relaxed);
    server_thread.join();
    close(listen_fd);

    std::cout << std::format("Success clients: {} / {}\n",
        success_count.load(std::memory_order_relaxed),
        NUM_TEST_CLIENTS);
    std::cout << std::format("Echoed count: {}\n", echoed_count.load(std::memory_order_relaxed));

    return 0;
}

```

## Ключевая структурная параллель между двумя файлами

Обе модели — completion-based, поэтому код структурно **параллелен строка в строку**, в отличие от epoll/WSAPoll:

|io_uring|IOCP|Смысл|
|---|---|---|
|`io_uring_queue_init()`|`CreateIoCompletionPort(INVALID_HANDLE_VALUE, ...)`|создать "порт" для результатов|
|`io_uring_prep_accept()` + `io_uring_sqe_set_data()`|`AcceptEx()` + `OVERLAPPED`|заказать асинхронный accept|
|`io_uring_wait_cqe_timeout()`|`GetQueuedCompletionStatus()`|заблокироваться до готового результата|
|`io_uring_cqe_get_data()`|`reinterpret_cast<IoContext*>(overlapped)`|достать "билет" завершённой операции|
|SQE user_data, живущий до CQE|`OVERLAPPED` как первое поле `IoContext`, тот же intrusive-приём|reclamation problem — нельзя освобождать, пока ядро держит ссылку|

Последняя строка таблицы — не случайность: и там, и там — ровно та же "reclamation problem", что разбирали для hazard pointers и broadcast-канала: память, на которую у ядра/ОС есть незавершённая асинхронная ссылка, нельзя освобождать до получения completion, иначе получите use-after-free, только уже не между вашими потоками, а между вашим кодом и ядром ОС.
