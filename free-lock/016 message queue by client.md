
### **Message queue на клиента**: SPSC от воркера к connection-writer (один producer — логика чата, один consumer — сетевой поток на отправку)

Реализую полную архитектуру: sharding клиентов по воркерам (гарантирует single producer на каждую per-client очередь) + SPSC-очередь на каждого клиента + отдельный writer-поток как единственный consumer.

```cpp
#include <atomic>
#include <iostream>
#include <format>
#include <thread>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <cstring>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <chrono>
#include <new>
#include <ranges>

namespace {
    // ============================================================
    // SPSC ring buffer -- простейшая lock-free очередь, Single
    // Producer Single Consumer. Никакого CAS вообще не нужно:
    // producer владеет ТОЛЬКО write_pos_, consumer -- ТОЛЬКО
    // read_pos_, каждый читает чужую позицию только через atomic
    // load для синхронизации видимости (happens-before), но никогда
    // не пишет в чужую -- поэтому конкуренции за одну и ту же
    // атомарную переменную с двух сторон просто нет.
    // ============================================================
    template <typename T, size_t Capacity>
    class SPSCQueue {
        static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be a power of 2");

        // for producer
        alignas(std::hardware_destructive_interference_size)
            std::atomic<size_t> write_pos_{0};
        // for consumer
        alignas(std::hardware_destructive_interference_size)
            std::atomic<size_t> read_pos_{0};

        T buffer_[Capacity];

        public:
            [[nodiscard]] bool push(T value) {
                const size_t w{write_pos_.load(std::memory_order_relaxed)};
                if (
                    const size_t r{read_pos_.load(std::memory_order_acquire)};
                    w - r >= Capacity) {
                    return false;
                }

                buffer_[w & (Capacity - 1)] = std::move(value);
                write_pos_.store(w + 1, std::memory_order_release);

                return true;
            }

            [[nodiscard]] bool pop(T& result) {
                const size_t r{read_pos_.load(std::memory_order_relaxed)};
                if (
                    const size_t w{write_pos_.load(std::memory_order_acquire)};
                    r == w) {
                    return false;
                }

                result = std::move(buffer_[r & (Capacity - 1)]);
                read_pos_.store(r + 1, std::memory_order_release);

                return true;
            }
    };

    // ============================================================
    // ClientConnection: несёт per-client исходящую SPSC-очередь.
    // Producer этой очереди -- ВСЕГДА один конкретный воркер
    // (гарантируется sharding'ом по fd ниже), consumer -- ВСЕГДА
    // один writer-поток.
    // ============================================================
    struct ClientConnection {
        int fd;
        SPSCQueue<std::string, 256> outbox;
        std::atomic<bool> active{true};
    };

    struct Task {
        int client_fd;
        std::string data;
    };

    // ============================================================
    // Реактор: принимает соединения, читает данные, ШАРДИРУЕТ
    // по fd между воркерами -- КАЖДЫЙ fd всегда попадает к ОДНОМУ
    // и тому же воркеру.
    // ============================================================
    class EpollReactor {
        int epoll_fd_;
        int listen_fd_;
        std::vector<SPSCQueue<Task, 1024>*>& workers_queues_;
        std::atomic<bool>& running_;
        int num_workers_;

        static bool set_nonblocking(const int fd) {
            const int flags{fcntl(fd, F_GETFL, 0)};
            if (flags == -1) return false;

            fcntl(fd, F_SETFL, flags | O_NONBLOCK);
            return true;
        }

    public:
        explicit EpollReactor(const int port,
                              std::vector<SPSCQueue<Task, 1024>*>& wq,
                              std::atomic<bool>& running):
            workers_queues_(wq),
            running_(running),
            num_workers_(static_cast<int>(wq.size())) {

            listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
            constexpr int opt{1};
            setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
            set_nonblocking(listen_fd_);

            sockaddr_in addr{};
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = INADDR_ANY;
            addr.sin_port = htons(port);
            running.store(running.load() && bind(listen_fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) >= 0);
            listen(listen_fd_, 128);
            epoll_fd_ = epoll_create1(0);
            epoll_event ev{};
            ev.events = EPOLLIN;
            ev.data.fd = listen_fd_;
            epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listen_fd_, &ev);
        }

        void run() {
            constexpr int MAX_EVENTS{64};
            epoll_event events[MAX_EVENTS];
            while (running_.load(std::memory_order_relaxed)) {
                const int n{epoll_wait(epoll_fd_, events, MAX_EVENTS, 100)};
                for (int i{}; i < n; ++i) {
                    if (events[i].data.fd == listen_fd_) {
                        const int client_fd{accept(listen_fd_, nullptr, nullptr)};
                        if (client_fd < 0) continue;
                        set_nonblocking(client_fd);
                        epoll_event cev{};
                        cev.events = EPOLLIN;
                        cev.data.fd = client_fd;
                        epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_fd, &cev);
                    } else {
                        const int fd{events[i].data.fd};
                        char buf[4096];
                        const ssize_t n_read{read(fd, buf, sizeof(buf))};
                        if (n_read <= 0) {
                            epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
                            continue;
                        }

                        // SHARDING: один и тот же fd ВСЕГДА идёт к одному воркеру.
                        const int worker_idx{fd % num_workers_};
                        if (Task task{.client_fd = fd, .data = std::string(buf, n_read)};
                            !workers_queues_[worker_idx]->push(std::move(task))) {
                            std::cerr
                                << std::format("[reactor] worker {} queue full, dropping\n", worker_idx)
                                << std::flush;
                        }
                    }
                }
            }

            close(listen_fd_);
            close(epoll_fd_);
        }
    };

    // ============================================================
    // Worker: читает СВОЮ SPSC-очередь задач (реактор -- единственный
    // producer), обрабатывает, и пишет ответ в SPSC outbox КОНКРЕТНОГО
    // клиента. Поскольку fd закреплён за этим воркером (sharding),
    // воркер -- ЕДИНСТВЕННЫЙ producer для outbox'а этого клиента.
    // ============================================================
    void worker_loop(const int worker_id,
                     SPSCQueue<Task, 1024>& input_queue,
                     std::unordered_map<int, std::shared_ptr<ClientConnection>>& clients,
                     std::mutex& clients_mutex,
                     const std::atomic<bool>& running,
                     std::atomic<long long>& processed) {
        Task task;
        while (true) {
            if (input_queue.pop(task)) {
                std::shared_ptr<ClientConnection> conn;
                {
                    std::lock_guard<std::mutex> lock{clients_mutex};
                    if (const auto it{clients.find(task.client_fd)};
                        it == clients.end()) {
                        conn = std::make_shared<ClientConnection>();
                        conn->fd = task.client_fd;
                        clients[task.client_fd] = conn;
                    } else {
                        conn = it->second;
                    }
                }

                if (const std::string response{std::format("[worker {}] echo: {}", worker_id, task.data)};
                    !conn->outbox.push(response)) {
                    std::cerr << std::format("[worker {}] outbox full for fd={}, dropping",
                        worker_id,
                        task.client_fd) << std::flush;
                }
                processed.fetch_add(1, std::memory_order_relaxed);
            } else {
                if (!running.load(std::memory_order_relaxed)) break;
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            }
        }
    }

    // ============================================================
    // Writer: ЕДИНСТВЕННЫЙ поток на всё приложение, вычитывающий
    // ВСЕ per-client outbox'ы и реально пишущий в сокеты.
    // ============================================================
    void writer_loop(std::unordered_map<int, std::shared_ptr<ClientConnection>>& clients,
                     std::mutex& clients_mutex,
                     const std::atomic<bool>& running,
                     std::atomic<long long>& written_count) {
        while (true) {
            bool did_work{false};
            std::vector<std::shared_ptr<ClientConnection>> snapshot;
            {
                std::lock_guard<std::mutex> lock{clients_mutex};
                snapshot.reserve(clients.size());
                for (auto& conn: clients | std::views::values) {
                    snapshot.push_back(conn);
                }
            }

            for (const auto& conn: snapshot) {
                std::string msg;
                while (conn->outbox.pop(msg)) {
                    const ssize_t n{write(conn->fd, msg.data(), msg.size())};
                    (void)n;
                    written_count.fetch_add(1, std::memory_order_relaxed);
                    did_work = true;
                }
            }

            if (!did_work) {
                if (!running.load(std::memory_order_relaxed)) break;
                std::this_thread::sleep_for(std::chrono::microseconds(200));
            }
        }
    }
}

int main(int argc, char *argv[]) {
    constexpr int PORT{18889};
    constexpr int NUM_WORKERS{4};
    constexpr int NUM_TEST_CLIENTS{20};

    std::atomic<bool> running{true};
    std::atomic<long long> processed{0};
    std::atomic<long long> written{0};

    std::vector<SPSCQueue<Task, 1024>> worker_input_storage(NUM_WORKERS);
    std::vector<SPSCQueue<Task, 1024>*> worker_input_ptrs;
    for (auto& q: worker_input_storage) worker_input_ptrs.push_back(&q);

    std::unordered_map<int, std::shared_ptr<ClientConnection>> clients;
    std::mutex clients_mutex;

    EpollReactor reactor(PORT, worker_input_ptrs, running);
    std::thread reactor_thread{[&reactor] { reactor.run(); }};

    std::vector<std::thread> workers;
    for (int w{}; w < NUM_WORKERS; ++w) {
        workers.emplace_back(
            worker_loop,
            w,
            std::ref(*worker_input_ptrs[w]),
            std::ref(clients),
            std::ref(clients_mutex),
            std::ref(running),
            std::ref(processed));
    }

    std::thread writer_thread{
        writer_loop,
        std::ref(clients),
        std::ref(clients_mutex),
        std::ref(running),
        std::ref(written)
    };
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::vector<std::thread> test_clients;
    std::atomic<int> success_count{0};
    for (int c{}; c < NUM_TEST_CLIENTS; ++c) {
        test_clients.emplace_back([&, c] {
            const int sock{socket(AF_INET, SOCK_STREAM, 0)};
            sockaddr_in addr{};
            addr.sin_family = AF_INET;
            addr.sin_port = htons(PORT);
            inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
            if (connect(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
                close(sock);
                return;
            }
            const std::string msg{std::format("hello from client: {}", c)};
            const ssize_t w{write(sock, msg.data(), msg.size())};
            (void)w;
            char buf[256] = {};
            if (const ssize_t n{read(sock, buf, sizeof(buf) - 1)};
                n > 0) {
                success_count.fetch_add(1, std::memory_order_relaxed);
            }
            close(sock);
        });
    }
    for (auto& t: test_clients) t.join();

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    running.store(false, std::memory_order_relaxed);

    reactor_thread.join();
    for (auto& t: workers) t.join();
    writer_thread.join();

    std::cout << std::format("Success clients: {} / {}\n", success_count.load(), NUM_TEST_CLIENTS);
    std::cout << std::format("Total processed workers: {}\n", processed.load());
    std::cout << std::format("Written by writer: {}\n", written.load());

    return 0;
}

```

### Ключевая архитектурная идея — sharding вместо MPSC

**Sharding по `fd % num_workers`** — это то, что превращает потенциальный MPSC (несколько воркеров могли бы писать одному клиенту) в честный SPSC: конкретный `fd` **навсегда** закреплён за одним воркером на всё время жизни соединения, поэтому только этот воркер когда-либо кладёт сообщения в `outbox` этого клиента. Как следствие, `SPSCQueue` — без единого CAS вообще, что дешевле любой MPSC/MPMC-структуры, разобранных раньше.

**Три уровня SPSC в этой архитектуре:**

1. Реактор → воркер: реактор один (единственный producer), воркер один (единственный consumer своей входной очереди).
2. Воркер → per-client outbox: конкретный воркер один (гарантировано sharding'ом), writer один.
3. **Writer — единственный на всё приложение**, кто когда-либо вызывает `write()` на сокет — это убирает гонку из предыдущей версии, где два воркера потенциально могли бы писать в один и тот же fd одновременно и порвать данные в сокете.

### Компромисс, который стоит понимать

**Sharding по `fd % N`** равномерно распределяет соединения при условии, что fd выделяются достаточно равномерно (обычно так и есть в Linux). Но если один "тяжёлый" клиент шлёт сообщения намного чаще остальных на том же воркере — этот воркер станет bottleneck'ом, а остальные будут простаивать (классический hot partition в шардированных системах). Для реального прода нужен либо hash по client ID с учётом load balancing, либо work-stealing между воркерами (что снова вернуло бы вопрос MPSC/MPMC).

**Единственный writer-поток** — тоже упрощение для наглядности: под большой нагрузкой (десятки тысяч соединений) один поток может не успевать проходить по всем outbox'ам достаточно часто. В продакшене обычно либо несколько writer-потоков (каждый со своим подмножеством клиентов через тот же sharding-принцип), либо интеграция с `EPOLLOUT` — писать не в busy-loop, а по сигналу "сокет готов принять данные".
