


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
!!!
```

```cpp
!!!
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

### Model 3: io_uring

**`io_uring_server.cpp` (Linux) — протестирован и стабилен.** По пути к этому результату поймал реальный баг: при повторных быстрых прогонах на одном порту `bind()` иногда падал с `Address already in use`, а код молча продолжал работать все отведённые секунды, ничего не принимая — выглядело как случайная гонка в io_uring, а оказалось банальным отсутствием проверки кода возврата. Добавил явные проверки `bind()`/`listen()`. После этого — 6 из 6 чистых прогонов (20/20 клиентов на разных портах) плюс 2 стабильных прогона подряд на одном порту с паузой.

**`iocp_server.cpp` (Windows) — НЕ скомпилирован и НЕ протестирован.** В этой песочнице нет Windows-тулчейна вообще, поэтому файл написан по документированному API (`AcceptEx`, `WSARecv`/`WSASend` с `OVERLAPPED`, `GetQueuedCompletionStatus`), но я не могу дать той же гарантии, что для Linux-версии. **Обязательно скомпилируйте и прогоните на своей машине**, прежде чем полагаться на него — там наверняка найдутся свои шероховатости (в частности, `iocp_server.cpp` не включает тестовых клиентов — только 5-секундный `sleep`, чтобы файл был компактнее; тестовую часть стоит перенести из io_uring-версии по аналогии, адаптировав под `SOCKET`/Winsock).

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


---
---

### epoll

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

#include <atomic>
#include <iostream>
#include <thread>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <string>
#include <chrono>
#include <new>

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

    inline void close_socket(socket_t s) { closesocket(s); }
    inline void set_nonblocking(socket_t s) {
        u_long mode = 1;
        ioctlsocket(s, FIONBIO, &mode);
    }

    struct WinsockInit {
        WinsockInit() {
            WSADATA wsa_data;
            if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
                throw std::runtime_error("WSAStartup failed");
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

inline int socket_read(socket_t s, char* buf, int len)  { return recv(s, buf, len, 0); }
inline int socket_write(socket_t s, const char* buf, int len) { return send(s, buf, len, 0); }

// ============================================================
// EventPoller: единый интерфейс поверх epoll (Linux) / WSAPoll (Windows).
// add()/remove() -- регистрация fd на чтение; wait() -- вернуть
// список fd, готовых к чтению, с таймаутом.
// ============================================================
#ifdef _WIN32

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
        fds_.erase(std::remove_if(fds_.begin(), fds_.end(),
                   [fd](const WSAPOLLFD& p) { return p.fd == fd; }), fds_.end());
    }

    // O(n) по числу зарегистрированных fd -- WSAPoll всегда сканирует
    // весь переданный массив, в отличие от epoll_wait
    std::vector<socket_t> wait(int timeout_ms) {
        std::vector<socket_t> ready;
        if (fds_.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms));
            return ready;
        }
        int n = WSAPoll(fds_.data(), static_cast<ULONG>(fds_.size()), timeout_ms);
        if (n <= 0) return ready;
        for (auto& pfd : fds_) {
            if (pfd.revents & (POLLRDNORM | POLLHUP | POLLERR)) {
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
class MPMCBus {
    struct Cell { std::atomic<size_t> sequence; T data; };
    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> enqueue_pos_;
    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> dequeue_pos_;
    Cell* buffer_;
    size_t buffer_mask_;
public:
    explicit MPMCBus(size_t capacity)
        : buffer_(new Cell[capacity]), buffer_mask_(capacity - 1) {
        for (size_t i = 0; i < capacity; ++i)
            buffer_[i].sequence.store(i, std::memory_order_relaxed);
        enqueue_pos_.store(0, std::memory_order_relaxed);
        dequeue_pos_.store(0, std::memory_order_relaxed);
    }
    ~MPMCBus() { delete[] buffer_; }
    MPMCBus(const MPMCBus&) = delete;

    bool try_push(T value) {
        Cell* cell; size_t pos = enqueue_pos_.load(std::memory_order_relaxed);
        for (;;) {
            cell = &buffer_[pos & buffer_mask_];
            size_t seq = cell->sequence.load(std::memory_order_acquire);
            intptr_t dif = (intptr_t)seq - (intptr_t)pos;
            if (dif == 0) { if (enqueue_pos_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) break; }
            else if (dif < 0) return false;
            else pos = enqueue_pos_.load(std::memory_order_relaxed);
        }
        cell->data = std::move(value);
        cell->sequence.store(pos + 1, std::memory_order_release);
        return true;
    }

    bool try_pop(T& result) {
        Cell* cell; size_t pos = dequeue_pos_.load(std::memory_order_relaxed);
        for (;;) {
            cell = &buffer_[pos & buffer_mask_];
            size_t seq = cell->sequence.load(std::memory_order_acquire);
            intptr_t dif = (intptr_t)seq - (intptr_t)(pos + 1);
            if (dif == 0) { if (dequeue_pos_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) break; }
            else if (dif < 0) return false;
            else pos = dequeue_pos_.load(std::memory_order_relaxed);
        }
        result = std::move(cell->data);
        cell->sequence.store(pos + buffer_mask_ + 1, std::memory_order_release);
        return true;
    }
};

struct Task { socket_t client_fd; std::string data; };

// ============================================================
// Reactor: единственный поток, крутит EventPoller::wait(), никогда
// не блокируется дольше таймаута. Логика идентична на обеих
// платформах -- разница спрятана внутри EventPoller.
// ============================================================
class Reactor {
    EventPoller poller_;
    socket_t listen_fd_;
    MPMCBus<Task>& task_queue_;
    std::atomic<bool>& running_;

public:
    Reactor(socket_t listen_fd, MPMCBus<Task>& queue, std::atomic<bool>& running)
        : listen_fd_(listen_fd), task_queue_(queue), running_(running) {
        poller_.add(listen_fd_);
    }

    void run() {
        while (running_.load(std::memory_order_relaxed)) {
            auto ready = poller_.wait(100);

            for (socket_t fd : ready) {
                if (fd == listen_fd_) {
                    socket_t client_fd = accept(listen_fd_, nullptr, nullptr);
                    if (client_fd == INVALID_SOCK) continue;
                    set_nonblocking(client_fd);
                    poller_.add(client_fd);
                } else {
                    char buf[4096];
                    int n_read = socket_read(fd, buf, sizeof(buf));
                    if (n_read <= 0) {
                        poller_.remove(fd);
                        close_socket(fd);
                        continue;
                    }
                    Task task{fd, std::string(buf, n_read)};
                    if (!task_queue_.try_push(std::move(task))) {
                        std::cerr << "[reactor] task queue full, dropping\n";
                    }
                }
            }
        }
    }
};

void worker_loop(int worker_id, MPMCBus<Task>& queue, std::atomic<bool>& running,
                  std::atomic<long long>& processed) {
    Task task;
    while (running.load(std::memory_order_relaxed) || true) {
        if (queue.try_pop(task)) {
            std::string response = "[worker " + std::to_string(worker_id) + "] echo: " + task.data;
            socket_write(task.client_fd, response.data(), static_cast<int>(response.size()));
            processed.fetch_add(1, std::memory_order_relaxed);
        } else {
            if (!running.load(std::memory_order_relaxed)) break;
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    }
}

int main() {
    constexpr int PORT = 18891;
    constexpr int NUM_WORKERS = 4;
    constexpr int NUM_TEST_CLIENTS = 20;

    WinsockInit winsock_guard;

    socket_t listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<const char*>(&opt), sizeof(opt));
    set_nonblocking(listen_fd);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    bind(listen_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    listen(listen_fd, 128);

    MPMCBus<Task> task_queue(1024);
    std::atomic<bool> running{true};
    std::atomic<long long> processed{0};

    Reactor reactor(listen_fd, task_queue, running);
    std::thread reactor_thread([&] { reactor.run(); });

    std::vector<std::thread> workers;
    for (int w = 0; w < NUM_WORKERS; ++w) {
        workers.emplace_back(worker_loop, w, std::ref(task_queue), std::ref(running), std::ref(processed));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::vector<std::thread> test_clients;
    std::atomic<int> success_count{0};
    for (int c = 0; c < NUM_TEST_CLIENTS; ++c) {
        test_clients.emplace_back([&, c] {
            socket_t sock = socket(AF_INET, SOCK_STREAM, 0);
            sockaddr_in caddr{};
            caddr.sin_family = AF_INET;
            caddr.sin_port = htons(PORT);
            inet_pton(AF_INET, "127.0.0.1", &caddr.sin_addr);

            if (connect(sock, reinterpret_cast<sockaddr*>(&caddr), sizeof(caddr)) != 0) {
                close_socket(sock);
                return;
            }
            std::string msg = "hello from client " + std::to_string(c);
            socket_write(sock, msg.data(), static_cast<int>(msg.size()));

            char buf[256] = {};
            int n = socket_read(sock, buf, sizeof(buf) - 1);
            if (n > 0) success_count.fetch_add(1, std::memory_order_relaxed);
            close_socket(sock);
        });
    }
    for (auto& t : test_clients) t.join();

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    running.store(false, std::memory_order_relaxed);

    reactor_thread.join();
    for (auto& t : workers) t.join();
    close_socket(listen_fd);

    std::cout << "Успешных клиентов: " << success_count.load() << " / " << NUM_TEST_CLIENTS << "\n";
    std::cout << "Задач обработано воркерами: " << processed.load() << "\n";

    return 0;
}

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
```

```
g++ -std=c++20 -O2 -pthread event_loop_demo.cpp -o event_loop_demo
./event_loop_demo
```


### io_uring 
```cpp
// ============================================================
// io_uring echo-сервер (Linux only) -- прямая демонстрация
// completion-based модели из теоретического разбора: submit
// SQE ("прочитай в этот буфер"), получить CQE ("готово, вот
// сколько байт"), без цикла readiness-опроса вообще.
//
// Сборка: g++ -std=c++20 -O2 -pthread io_uring_server.cpp -o server -luring
// ============================================================

#include <liburing.h>
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>
#include <unordered_map>
#include <cstring>
#include <cerrno>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <chrono>

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
        auto* op = new Operation{OpType::Accept, listen_fd_, {}};
        in_flight_[op] = std::unique_ptr<Operation>(op);

        io_uring_sqe* sqe = io_uring_get_sqe(&ring_);
        io_uring_prep_accept(sqe, listen_fd_, nullptr, nullptr, 0);
        io_uring_sqe_set_data(sqe, op);
    }

    void submit_read(int fd) {
        auto* op = new Operation{OpType::Read, fd, std::vector<char>(4096)};
        in_flight_[op] = std::unique_ptr<Operation>(op);

        io_uring_sqe* sqe = io_uring_get_sqe(&ring_);
        io_uring_prep_read(sqe, fd, op->buffer.data(), op->buffer.size(), 0);
        io_uring_sqe_set_data(sqe, op);
    }

    void submit_write(int fd, std::string data) {
        auto* op = new Operation{OpType::Write, fd, {}};
        op->buffer.assign(data.begin(), data.end());
        in_flight_[op] = std::unique_ptr<Operation>(op);

        io_uring_sqe* sqe = io_uring_get_sqe(&ring_);
        io_uring_prep_write(sqe, fd, op->buffer.data(), op->buffer.size(), 0);
        io_uring_sqe_set_data(sqe, op);
    }

public:
    IoUringServer(int listen_fd, std::atomic<bool>& running, std::atomic<long long>& echoed)
        : listen_fd_(listen_fd), running_(running), echoed_count_(echoed) {
        io_uring_queue_init(256, &ring_, 0);
    }

    ~IoUringServer() { io_uring_queue_exit(&ring_); }

    void run() {
        submit_accept();
        io_uring_submit(&ring_);

        while (running_.load(std::memory_order_relaxed)) {
            io_uring_cqe* cqe;
            __kernel_timespec timeout{0, 100'000'000}; // 100мс -- проверять running периодически
            int ret = io_uring_wait_cqe_timeout(&ring_, &cqe, &timeout);
            if (ret == -ETIME) continue; // таймаут -- просто снова проверяем running
            if (ret < 0) continue;

            auto* op = static_cast<Operation*>(io_uring_cqe_get_data(cqe));
            int result = cqe->res;
            io_uring_cqe_seen(&ring_, cqe);

            switch (op->type) {
                case OpType::Accept: {
                    if (result >= 0) {
                        int client_fd = result;
                        submit_read(client_fd); // сразу заказываем чтение с нового клиента
                    }
                    submit_accept(); // и сразу снова слушаем следующее подключение
                    break;
                }
                case OpType::Read: {
                    if (result <= 0) {
                        close(op->fd); // клиент отключился или ошибка
                    } else {
                        std::string request(op->buffer.data(), result);
                        std::string response = "[io_uring] echo: " + request;
                        submit_write(op->fd, std::move(response));
                    }
                    break;
                }
                case OpType::Write: {
                    if (result > 0) {
                        echoed_count_.fetch_add(1, std::memory_order_relaxed);
                        submit_read(op->fd); // после ответа -- снова ждём следующее сообщение
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

int main() {
    constexpr int PORT = 18892;
    constexpr int NUM_TEST_CLIENTS = 20;

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        std::cerr << "socket() failed: " << strerror(errno) << "\n";
        return 1;
    }
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    if (bind(listen_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        // ВАЖНО: если запускаете этот сервер повторно на том же порту
        // сразу друг за другом (как я делал при отладке), возможна
        // "Address already in use" -- SO_REUSEADDR не всегда мгновенно
        // освобождает порт. Явная проверка здесь -- не паранойя, а то,
        // из-за отсутствия чего я сам словил "фантомную" нестабильность
        // при тестировании: без проверки код тихо продолжал бы работать
        // все отведённые секунды, ничего не принимая, и выглядело бы
        // как случайная гонка в io_uring, а не как обычная ошибка bind().
        std::cerr << "bind() failed: " << strerror(errno) << "\n";
        return 1;
    }
    if (listen(listen_fd, 128) < 0) {
        std::cerr << "listen() failed: " << strerror(errno) << "\n";
        return 1;
    }

    std::atomic<bool> running{true};
    std::atomic<long long> echoed_count{0};

    IoUringServer server(listen_fd, running, echoed_count);
    std::thread server_thread([&] { server.run(); });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::vector<std::thread> test_clients;
    std::atomic<int> success_count{0};
    for (int c = 0; c < NUM_TEST_CLIENTS; ++c) {
        test_clients.emplace_back([&, c] {
            int sock = socket(AF_INET, SOCK_STREAM, 0);
            sockaddr_in caddr{};
            caddr.sin_family = AF_INET;
            caddr.sin_port = htons(PORT);
            inet_pton(AF_INET, "127.0.0.1", &caddr.sin_addr);

            if (connect(sock, (sockaddr*)&caddr, sizeof(caddr)) < 0) {
                close(sock);
                return;
            }
            std::string msg = "hello from client " + std::to_string(c);
            write(sock, msg.data(), msg.size());

            char buf[256] = {};
            ssize_t n = read(sock, buf, sizeof(buf) - 1);
            if (n > 0) success_count.fetch_add(1, std::memory_order_relaxed);
            close(sock);
        });
    }
    for (auto& t : test_clients) t.join();

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    running.store(false, std::memory_order_relaxed);
    server_thread.join();
    close(listen_fd);

    std::cout << "Успешных клиентов: " << success_count.load() << " / " << NUM_TEST_CLIENTS << "\n";
    std::cout << "Echo обработано: " << echoed_count.load() << "\n";

    return 0;
}
```

### iocp
```cpp
// ============================================================
// IOCP echo-сервер (Windows only) -- completion-based аналог
// io_uring_server.cpp. Концептуально та же модель ("submit async
// op -> получить completion позже"), но другой API и другая
// внутренняя механика (нет разделяемых SQ/CQ колец в userspace --
// вместо этого单 GetQueuedCompletionStatus() вытягивает готовые
// результаты из очереди, которую ведёт ядро).
//
// Сборка (MSVC): cl /std:c++20 /EHsc iocp_server.cpp /link ws2_32.lib
// Сборка (MinGW): g++ -std=c++20 -O2 iocp_server.cpp -o server.exe -lws2_32 -lmswsock
// ============================================================

#ifndef _WIN32
#error "Этот файл собирается только на Windows -- см. io_uring_server.cpp для Linux"
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
#include <thread>
#include <vector>
#include <string>

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
    SOCKET client_fd = INVALID_SOCKET;
    std::vector<char> buffer;
    WSABUF wsabuf{};
};

std::atomic<long long> echoed_count{0};
std::atomic<bool> running{true};

LPFN_ACCEPTEX AcceptExPtr = nullptr;

void load_acceptex(SOCKET listen_fd) {
    GUID guid = WSAID_ACCEPTEX;
    DWORD bytes = 0;
    WSAIoctl(listen_fd, SIO_GET_EXTENSION_FUNCTION_POINTER,
              &guid, sizeof(guid), &AcceptExPtr, sizeof(AcceptExPtr),
              &bytes, nullptr, nullptr);
}

void submit_accept(HANDLE iocp, SOCKET listen_fd) {
    SOCKET accept_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP,
                                       nullptr, 0, WSA_FLAG_OVERLAPPED);

    auto* ctx = new IoContext();
    ctx->type = OpType::Accept;
    ctx->client_fd = accept_socket;
    ctx->buffer.resize(64); // под адреса, требуемые AcceptEx

    DWORD bytes_received = 0;
    // AcceptEx -- асинхронный аналог accept(): принимает соединение
    // БЕЗ блокировки, результат придёт как IOCP completion, точно
    // так же, как io_uring_prep_accept + последующий CQE
    BOOL ok = AcceptExPtr(listen_fd, accept_socket,
                            ctx->buffer.data(), 0,
                            sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
                            &bytes_received, &ctx->overlapped);

    if (!ok && WSAGetLastError() != ERROR_IO_PENDING) {
        std::cerr << "AcceptEx failed: " << WSAGetLastError() << "\n";
        delete ctx;
        closesocket(accept_socket);
    }
    // Если ok==FALSE и ERROR_IO_PENDING -- это НЕ ошибка, это нормальное
    // "операция поставлена в очередь, результат придёт через IOCP" --
    // ровно та же семантика, что "SQE отправлен, ждём CQE" в io_uring
}

void submit_read(HANDLE iocp, SOCKET client_fd) {
    auto* ctx = new IoContext();
    ctx->type = OpType::Read;
    ctx->client_fd = client_fd;
    ctx->buffer.resize(4096);
    ctx->wsabuf.buf = ctx->buffer.data();
    ctx->wsabuf.len = static_cast<ULONG>(ctx->buffer.size());

    DWORD flags = 0;
    DWORD bytes = 0;
    int ret = WSARecv(client_fd, &ctx->wsabuf, 1, &bytes, &flags,
                        &ctx->overlapped, nullptr);

    if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        delete ctx;
        closesocket(client_fd);
    }
}

void submit_write(HANDLE iocp, SOCKET client_fd, std::string data) {
    auto* ctx = new IoContext();
    ctx->type = OpType::Write;
    ctx->client_fd = client_fd;
    ctx->buffer.assign(data.begin(), data.end());
    ctx->wsabuf.buf = ctx->buffer.data();
    ctx->wsabuf.len = static_cast<ULONG>(ctx->buffer.size());

    DWORD bytes = 0;
    int ret = WSASend(client_fd, &ctx->wsabuf, 1, &bytes, 0,
                        &ctx->overlapped, nullptr);

    if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        delete ctx;
        closesocket(client_fd);
    }
}

void iocp_loop(HANDLE iocp, SOCKET listen_fd) {
    while (running.load(std::memory_order_relaxed)) {
        DWORD bytes_transferred = 0;
        ULONG_PTR completion_key = 0;
        OVERLAPPED* overlapped = nullptr;

        // GetQueuedCompletionStatus -- прямой аналог io_uring_wait_cqe_timeout:
        // блокируется до готового результата ИЛИ таймаута (100мс, чтобы
        // периодически проверять running -- тот же приём, что и в io_uring
        // версии, и в epoll-версии до этого)
        BOOL ok = GetQueuedCompletionStatus(iocp, &bytes_transferred,
                                              &completion_key, &overlapped, 100);

        if (overlapped == nullptr) continue; // таймаут -- проверяем running и продолжаем

        // Тот самый intrusive-трюк: overlapped -- это первое поле IoContext,
        // поэтому адрес overlapped == адрес IoContext
        auto* ctx = reinterpret_cast<IoContext*>(overlapped);

        if (!ok || bytes_transferred == 0) {
            if (ctx->type != OpType::Accept) closesocket(ctx->client_fd);
            delete ctx;
            continue;
        }

        switch (ctx->type) {
            case OpType::Accept: {
                CreateIoCompletionPort(reinterpret_cast<HANDLE>(ctx->client_fd),
                                         iocp, 0, 0);
                submit_read(iocp, ctx->client_fd);
                submit_accept(iocp, listen_fd); // сразу снова слушаем следующее подключение
                delete ctx;
                break;
            }
            case OpType::Read: {
                std::string request(ctx->buffer.data(), bytes_transferred);
                std::string response = "[iocp] echo: " + request;
                SOCKET fd = ctx->client_fd;
                delete ctx;
                submit_write(iocp, fd, std::move(response));
                break;
            }
            case OpType::Write: {
                echoed_count.fetch_add(1, std::memory_order_relaxed);
                SOCKET fd = ctx->client_fd;
                delete ctx;
                submit_read(iocp, fd); // ждём следующее сообщение от этого клиента
                break;
            }
        }
    }
}

int main() {
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2, 2), &wsa_data);

    constexpr int PORT = 18893;

    SOCKET listen_fd = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP,
                                    nullptr, 0, WSA_FLAG_OVERLAPPED);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(listen_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        std::cerr << "bind() failed: " << WSAGetLastError() << "\n";
        return 1;
    }
    listen(listen_fd, 128);

    load_acceptex(listen_fd);

    // IOCP handle -- аналог io_uring_queue_init(): создаём "порт
    // завершения", к которому потом привязываем listen_fd и каждый
    // новый client_fd через CreateIoCompletionPort
    HANDLE iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
    CreateIoCompletionPort(reinterpret_cast<HANDLE>(listen_fd), iocp, 0, 0);

    submit_accept(iocp, listen_fd);

    std::thread iocp_thread(iocp_loop, iocp, listen_fd);

    // ... здесь были бы тестовые клиенты, аналогично io_uring_server.cpp ...
    std::this_thread::sleep_for(std::chrono::seconds(5));

    running.store(false, std::memory_order_relaxed);
    iocp_thread.join();

    std::cout << "Echo обработано: " << echoed_count.load() << "\n";

    closesocket(listen_fd);
    CloseHandle(iocp);
    WSACleanup();
    return 0;
}
```

---
---

## Блок 1 — Фундамент: memory model и atomics (3-5 дней)

**Теория:**

- [x] `std::memory_order`: relaxed, acquire, release, acq_rel, seq_cst — что каждый реально гарантирует на уровне happens-before (2026.09.14)
- [x] Разница между atomicity и ordering  (2026.09.14)
- [x] ABA problem и способы борьбы (tagged pointers, hazard pointers, epoch-based reclamation)  (2026.09.15)
- [x] False sharing и cache line padding (`alignas(64)`, `std::hardware_destructive_interference_size`)  (2026.09.15)
- [x] CAS (compare_exchange_weak vs strong) — почему weak предпочтителен в циклах (2026.09.15)

**Практика:**

- [x] Написать spinlock на atomic_flag, замерить под контейнером с perf (2026.09.17)
- [x] Реализовать простой atomic counter с разными memory_order и посмотреть разницу в асме (godbolt) (2026.09.17)
- [x] Разобрать пример ABA на указателях вручную (2026.09.18)

**Источники:** "C++ Concurrency in Action" (Anthony Williams) главы 5, 7; CppCon talks Herb Sutter "atomic<> Weapons"; cppreference по memory_order.

## Блок 2 — Lock-free структуры данных (1-2 недели)

**Порядок изучения (от простого к сложному):**

1. [x] **MPSC queue** (Michael-Scott или на основе intrusive linked list) — уже ближе к реальному broadcast-паттерну (2026.09.18)
2. [x] **MPMC queue** (Dmitry Vyukov's bounded queue — классика, часто спрашивают на собесах) (2026.09.18)
3. [x] Lock-free stack (Treiber stack) — проще MPMC, хорошо иллюстрирует ABA (2026.09.18)

**Для каждой структуры:**

- [x] Реализовать самому на raw atomics (не подглядывая) (2026.09.21)
- [x] Написать stress-test с несколькими потоками + TSan (ThreadSanitizer) — это критично, lock-free код без санитайзера почти невозможно верифицировать (2026.09.21)
- [x] Сравнить throughput/latency с mutex-based аналогом (тот же интерфейс, `std::mutex + std::deque`) (2026.09.21)

**Источники:** Dmitry Vyukov's blog (1024cores.net) — обязательно; "The Art of Multiprocessor Programming" (Herlihy & Shavit) главы про queues/stacks; boost::lockfree как референс-реализация для сверки.

## Блок 3 — Архитектура lock-free чат-сервера (1 неделя)

**Ключевые решения для чата конкретно:**

- [ ] **Модель потоков**: thread-per-connection vs event loop (epoll/io_uring) + пул воркеров — lock-free структуры чаще всего нужны именно во второй модели, как shared-state между воркерами
- [ ] **Message queue на клиента**: SPSC от воркера к connection-writer (один producer — логика чата, один consumer — сетевой поток на отправку)
- [ ] **Broadcast**: тут MPSC/MPMC не всегда лучший выбор — часто эффективнее copy-on-write список подписчиков (RCU-style) + каждому подписчику своя SPSC-очередь, чем один общий lock-free broadcast-канал
- [ ] **Backpressure**: что делать, если consumer медленнее producer — bounded queue + drop policy vs unbounded + memory blowup
- [ ] **Reclamation проблема**: если пользователь отключается, когда безопасно освободить его структуры при lock-free доступе (hazard pointers / epoch-based / RCU)

**Практика:** собрать минимальный прототип — N потоков-писателей кладут сообщения в шину, M потоков-читателей (эмулируют коннекты) их разбирают, с метриками latency/throughput под нагрузкой.

**Источники:** статьи про архитектуру Discord/Slack backend (общие принципы, не lock-free специфика, но полезно для контекста), RCU paper (Paul McKenney), доклады про io_uring + lock-free от CppCon/Meeting C++.

----
---
---
---
---
---
---

### **Message queue на клиента**: SPSC от воркера к connection-writer (один producer — логика чата, один consumer — сетевой поток на отправку)

Реализую полную архитектуру: sharding клиентов по воркерам (гарантирует single producer на каждую per-client очередь) + SPSC-очередь на каждого клиента + отдельный writer-поток как единственный consumer.

Работает: 20/20 клиентов, воркеры и writer обработали по 20 сообщений. Уберу неиспользуемую функцию `worker_loop` (в `main` логику пришлось инлайнить лямбдой из-за регистрации клиента) — почищу перед финальным показом.

```cpp
#include <atomic>
#include <iostream>
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

// ============================================================
// SPSC ring buffer -- простейшая lock-free очередь, Single
// Producer Single Consumer. Никакого CAS вообще не нужно:
// producer владеет ТОЛЬКО write_pos_, consumer -- ТОЛЬКО
// read_pos_, каждый читает чужую позицию только через atomic
// load для синхронизации видимости (happens-before), но никогда
// не пишет в чужую -- поэтому конкуренции за одну и ту же
// атомарную переменную с двух сторон просто нет.
// ============================================================
template<typename T, size_t Capacity>
class SPSCQueue {
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be power of 2");

    alignas(std::hardware_destructive_interference_size)
        std::atomic<size_t> write_pos_{0}; // пишет ТОЛЬКО producer
    alignas(std::hardware_destructive_interference_size)
        std::atomic<size_t> read_pos_{0};  // пишет ТОЛЬКО consumer

    T buffer_[Capacity];

public:
    bool push(T value) {
        size_t w = write_pos_.load(std::memory_order_relaxed);
        size_t r = read_pos_.load(std::memory_order_acquire);
        if (w - r >= Capacity) return false;

        buffer_[w & (Capacity - 1)] = std::move(value);
        write_pos_.store(w + 1, std::memory_order_release);
        return true;
    }

    bool pop(T& result) {
        size_t r = read_pos_.load(std::memory_order_relaxed);
        size_t w = write_pos_.load(std::memory_order_acquire);
        if (r == w) return false;

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

struct Task { int client_fd; std::string data; };

// ============================================================
// Реактор: принимает соединения, читает данные, ШАРДИРУЕТ
// по fd между воркерами -- КАЖДЫЙ fd всегда попадает к ОДНОМУ
// и тому же воркеру.
// ============================================================
class EpollReactor {
    int epoll_fd_, listen_fd_;
    std::vector<SPSCQueue<Task, 1024>*>& worker_queues_;
    std::atomic<bool>& running_;
    int num_workers_;

    static void set_nonblocking(int fd) {
        int flags = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }

public:
    EpollReactor(int port, std::vector<SPSCQueue<Task, 1024>*>& wq, std::atomic<bool>& running)
        : worker_queues_(wq), running_(running), num_workers_(wq.size()) {
        listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        int opt = 1;
        setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        set_nonblocking(listen_fd_);
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);
        bind(listen_fd_, (sockaddr*)&addr, sizeof(addr));
        listen(listen_fd_, 128);
        epoll_fd_ = epoll_create1(0);
        epoll_event ev{}; ev.events = EPOLLIN; ev.data.fd = listen_fd_;
        epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listen_fd_, &ev);
    }

    void run() {
        constexpr int MAX_EVENTS = 64;
        epoll_event events[MAX_EVENTS];
        while (running_.load(std::memory_order_relaxed)) {
            int n = epoll_wait(epoll_fd_, events, MAX_EVENTS, 100);
            for (int i = 0; i < n; ++i) {
                if (events[i].data.fd == listen_fd_) {
                    int client_fd = accept(listen_fd_, nullptr, nullptr);
                    if (client_fd < 0) continue;
                    set_nonblocking(client_fd);
                    epoll_event cev{}; cev.events = EPOLLIN; cev.data.fd = client_fd;
                    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_fd, &cev);
                } else {
                    int fd = events[i].data.fd;
                    char buf[4096];
                    ssize_t n_read = read(fd, buf, sizeof(buf));
                    if (n_read <= 0) {
                        epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
                        continue;
                    }
                    // SHARDING: один и тот же fd ВСЕГДА идёт к одному воркеру.
                    int worker_idx = fd % num_workers_;
                    Task task{fd, std::string(buf, n_read)};
                    if (!worker_queues_[worker_idx]->push(std::move(task))) {
                        std::cerr << "[reactor] worker " << worker_idx << " queue full, dropping\n";
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
void worker_loop(int worker_id, SPSCQueue<Task, 1024>& input_queue,
                  std::unordered_map<int, std::shared_ptr<ClientConnection>>& clients,
                  std::mutex& clients_mutex,
                  std::atomic<bool>& running, std::atomic<long long>& processed) {
    Task task;
    while (running.load(std::memory_order_relaxed) || true) {
        if (input_queue.pop(task)) {
            std::shared_ptr<ClientConnection> conn;
            {
                std::lock_guard<std::mutex> lock(clients_mutex);
                auto it = clients.find(task.client_fd);
                if (it == clients.end()) {
                    conn = std::make_shared<ClientConnection>();
                    conn->fd = task.client_fd;
                    clients[task.client_fd] = conn;
                } else {
                    conn = it->second;
                }
            }

            std::string response = "[worker " + std::to_string(worker_id) + "] echo: " + task.data;
            if (!conn->outbox.push(response)) {
                std::cerr << "[worker " << worker_id << "] outbox full for fd="
                          << task.client_fd << ", dropping\n";
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
                  std::mutex& clients_mutex, std::atomic<bool>& running,
                  std::atomic<long long>& written_count) {
    while (running.load(std::memory_order_relaxed) || true) {
        bool did_work = false;
        std::vector<std::shared_ptr<ClientConnection>> snapshot;
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            snapshot.reserve(clients.size());
            for (auto& [fd, conn] : clients) snapshot.push_back(conn);
        }

        for (auto& conn : snapshot) {
            std::string msg;
            while (conn->outbox.pop(msg)) {
                ssize_t n = write(conn->fd, msg.data(), msg.size());
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

int main() {
    constexpr int PORT = 18889;
    constexpr int NUM_WORKERS = 4;
    constexpr int NUM_TEST_CLIENTS = 20;

    std::atomic<bool> running{true};
    std::atomic<long long> processed{0}, written{0};

    std::vector<SPSCQueue<Task, 1024>> worker_input_storage(NUM_WORKERS);
    std::vector<SPSCQueue<Task, 1024>*> worker_input_ptrs;
    for (auto& q : worker_input_storage) worker_input_ptrs.push_back(&q);

    std::unordered_map<int, std::shared_ptr<ClientConnection>> clients;
    std::mutex clients_mutex;

    EpollReactor reactor(PORT, worker_input_ptrs, running);
    std::thread reactor_thread([&] { reactor.run(); });

    std::vector<std::thread> workers;
    for (int w = 0; w < NUM_WORKERS; ++w) {
        workers.emplace_back(worker_loop, w, std::ref(*worker_input_ptrs[w]),
                              std::ref(clients), std::ref(clients_mutex),
                              std::ref(running), std::ref(processed));
    }

    std::thread writer_thread(writer_loop, std::ref(clients), std::ref(clients_mutex),
                               std::ref(running), std::ref(written));

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::vector<std::thread> test_clients;
    std::atomic<int> success_count{0};
    for (int c = 0; c < NUM_TEST_CLIENTS; ++c) {
        test_clients.emplace_back([&, c] {
            int sock = socket(AF_INET, SOCK_STREAM, 0);
            sockaddr_in addr{};
            addr.sin_family = AF_INET;
            addr.sin_port = htons(PORT);
            inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
            if (connect(sock, (sockaddr*)&addr, sizeof(addr)) < 0) { close(sock); return; }
            std::string msg = "hello from client " + std::to_string(c);
            ssize_t w = write(sock, msg.data(), msg.size()); (void)w;
            char buf[256] = {};
            ssize_t n = read(sock, buf, sizeof(buf) - 1);
            if (n > 0) success_count.fetch_add(1, std::memory_order_relaxed);
            close(sock);
        });
    }
    for (auto& t : test_clients) t.join();

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    running.store(false, std::memory_order_relaxed);

    reactor_thread.join();
    for (auto& t : workers) t.join();
    writer_thread.join();

    std::cout << "Успешных клиентов: " << success_count.load() << " / " << NUM_TEST_CLIENTS << "\n";
    std::cout << "Обработано воркерами: " << processed.load() << "\n";
    std::cout << "Записано writer'ом: " << written.load() << "\n";

    return 0;
}
```

```
g++ -std=c++20 -O2 -pthread spsc_writer_demo.cpp -o spsc_writer_demo
./spsc_writer_demo
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

Готовы перейти к **broadcast** (следующий пункт плана — copy-on-write список подписчиков + RCU-style рассылка), или сначала добавить в этот код backpressure для outbox (сейчас просто "drop и лог в cerr")?

---
---

### **Broadcast**: тут MPSC/MPMC не всегда лучший выбор — часто эффективнее copy-on-write список подписчиков (RCU-style) + каждому подписчику своя SPSC-очередь, чем один общий lock-free broadcast-канал


Реализую broadcast-канал на copy-on-write списке подписчиков (RCU-style read) — каждый подписчик получает сообщение через свою SPSC-очередь, без единого общего lock-free канала.Good — процесс не завис. Причина зависания найдена: bounded SPSC-очередь (capacity 1024) переполняется под 80 000 сообщений, `publish()` тихо дропает лишние, а consumer ждёт получить ровно `TOTAL_MESSAGES` — которого никогда не наберётся. Исправляю: `publish()` делает retry вместо тихого дропа (backpressure, как в предыдущих демках), плюс отдельно уберу лишний `|| true`.Важный момент, который стоит проговорить явно: в моём первом варианте теста 4 publisher-потока **одновременно** писали в один и тот же `inbox` подписчика — а `SPSCQueue` рассчитан ровно на одного producer'а. С несколькими параллельными publisher'ами это уже реальная гонка (multiple producers на "single producer" структуру), а не гипотетическая. Исправляю тест на архитектурно корректный: **один поток владеет топиком** и публикует последовательно — это и есть условие, при котором `SPSC` на подписчика вообще имеет право быть SPSC.Теперь пересоберу с таймаутом на выполнение, чтобы избежать повторного зависания при наличии остаточного бага.Нашёл настоящую причину зависания — это важный практический урок, а не просто баг демки. Поправлю архитектурно правильно.Оба теста прошли: тест 1 — точная доставка 80 000/80 000 всем 5 подписчикам, тест 2 — 165 201 публикация под непрерывным churn (216 725 subscribe/unsubscribe) без единого зависания. Один drop в логе — ровно ожидаемое поведение bounded-retry policy на пограничном случае. Проверю на TSan:TSan чист. Теперь уберу отладочный маркер и покажу финальный код.Итоговый рабочий код — прошёл функциональный тест (80 000/80 000 доставлено) и TSan без единого предупреждения:

```cpp
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>
#include <memory>
#include <chrono>
#include <new>

// ============================================================
// SPSC ring buffer -- один producer, один consumer на подписчика
// ============================================================
template<typename T, size_t Capacity>
class SPSCQueue {
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be power of 2");
    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> write_pos_{0};
    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> read_pos_{0};
    T buffer_[Capacity];
public:
    bool push(T value) {
        size_t w = write_pos_.load(std::memory_order_relaxed);
        size_t r = read_pos_.load(std::memory_order_acquire);
        if (w - r >= Capacity) return false;
        buffer_[w & (Capacity - 1)] = std::move(value);
        write_pos_.store(w + 1, std::memory_order_release);
        return true;
    }
    bool pop(T& result) {
        size_t r = read_pos_.load(std::memory_order_relaxed);
        size_t w = write_pos_.load(std::memory_order_acquire);
        if (r == w) return false;
        result = std::move(buffer_[r & (Capacity - 1)]);
        read_pos_.store(r + 1, std::memory_order_release);
        return true;
    }
};

struct Subscriber {
    int id;
    SPSCQueue<std::string, 1024> inbox;
    std::atomic<long long> received{0};
};

// ============================================================
// BroadcastChannel: copy-on-write список подписчиков (RCU-style)
// ============================================================
class BroadcastChannel {
public:
    using SubscriberList = std::vector<std::shared_ptr<Subscriber>>;

private:
    std::atomic<std::shared_ptr<const SubscriberList>> subscribers_;

public:
    BroadcastChannel() {
        subscribers_.store(std::make_shared<const SubscriberList>());
    }

    void subscribe(std::shared_ptr<Subscriber> sub) {
        std::shared_ptr<const SubscriberList> old_list, new_list;
        do {
            old_list = subscribers_.load(std::memory_order_acquire);
            auto copy = std::make_shared<SubscriberList>(*old_list);
            copy->push_back(sub);
            new_list = std::move(copy);
        } while (!subscribers_.compare_exchange_weak(
                     old_list, new_list,
                     std::memory_order_acq_rel, std::memory_order_acquire));
    }

    void unsubscribe(int subscriber_id) {
        std::shared_ptr<const SubscriberList> old_list, new_list;
        do {
            old_list = subscribers_.load(std::memory_order_acquire);
            auto copy = std::make_shared<SubscriberList>();
            copy->reserve(old_list->size());
            for (auto& s : *old_list) {
                if (s->id != subscriber_id) copy->push_back(s);
            }
            new_list = std::move(copy);
        } while (!subscribers_.compare_exchange_weak(
                     old_list, new_list,
                     std::memory_order_acq_rel, std::memory_order_acquire));
    }

    // ВАЖНО: если подписчик отписался, но publisher уже держит
    // старый snapshot, он всё ещё видит этот shared_ptr<Subscriber>.
    // Без границы retry зависший/отключённый подписчик заблокировал
    // бы доставку ВСЕМ остальным -- отсюда MAX_RETRY + drop.
    void publish(const std::string& message) {
        std::shared_ptr<const SubscriberList> snapshot =
            subscribers_.load(std::memory_order_acquire);

        constexpr int MAX_RETRY = 1000;
        for (const auto& sub : *snapshot) {
            int attempts = 0;
            while (!sub->inbox.push(message)) {
                if (++attempts >= MAX_RETRY) {
                    std::cerr << "[broadcast] subscriber " << sub->id
                              << " not draining, dropping message\n";
                    break;
                }
                std::this_thread::yield();
            }
        }
    }
};
```

## Ключевая идея — почему COW + per-subscriber SPSC лучше общего broadcast-канала

**Общий MPMC/MPSC broadcast-канал** означал бы: все подписчики читают из **одной** структуры, и там нужна была бы либо фильтрация "это сообщение не моё" на каждого читателя, либо сложная схема с reference counting на каждое сообщение (сколько подписчиков ещё не прочитали — когда все прочитали, можно освобождать). Это создаёт **contention между самими подписчиками** за общую структуру — ровно то, чего лучше избегать.

**COW-список + persubscriber SPSC** разносит это на два независимых механизма:

- `publish()` — просто **один `atomic load`** shared_ptr'а (снимок списка), затем независимая работа с каждым SPSC — **никакого contention между publish-вызовами и между подписчиками друг с другом**.
- `subscribe`/`unsubscribe` — редкие операции, единственные, что платят цену (копирование всего вектора + CAS-retry), что приемлемо, потому что подключение/отключение случается на порядки реже, чем broadcast сообщений.

## RCU-семантика — "eventually consistent" членство

Snapshot, который держит `publish()`, может быть на долю секунды устаревшим относительно только что вызванного `subscribe()`/`unsubscribe()` — это **осознанный компромисс**, а не баг: новый подписчик может не получить одно-два сообщения, отправленных прямо в момент его подключения, а отписавшийся может получить одно лишнее после `unsubscribe()`. Для broadcast-паттерна (presence, статусы, чат-сообщения с идемпотентной доставкой) это обычно приемлемо; там, где нужна строгая консистентность членства, RCU-подход не годится.

## Реальный баг, который я поймал при тестировании (стоит знать заранее)

Первая версия `publish()` с **бесконечным retry** зависала намертво: если подписчик отписывался, пока publisher ещё держал старый snapshot с ссылкой на него, publisher вечно пытался писать в очередь, которую больше никто не читает — **и блокировал доставку всем остальным подписчикам** (publish идёт последовательно по списку). `MAX_RETRY` + drop с логом — обязательная защита, не опциональная оптимизация, для любой системы, где consumer может исчезнуть, пока producer держит на него ссылку.

**Сборка и тесты:**

```bash
g++ -std=c++20 -O2 -pthread broadcast_cow.cpp -o broadcast_cow -latomic
./broadcast_cow

g++ -std=c++20 -O0 -g -fsanitize=thread -pthread broadcast_cow.cpp -o broadcast_cow_tsan -latomic
./broadcast_cow_tsan
```

Готовы перейти к **Backpressure** (следующий пункт Блока 3 — уже частично затронут через `MAX_RETRY`, но стоит формализовать policy: drop oldest vs drop newest vs disconnect), или к **Reclamation problem** (что происходит с памятью подписчика при `unsubscribe`, пока publisher ещё держит на него shared_ptr — мы полагались на refcount, но стоит явно разобрать, чем это отличается от hazard pointers для этого конкретного случая)?


---
---

### **Backpressure**: что делать, если consumer медленнее producer — bounded queue + drop policy vs unbounded + memory blowup

Вижу два момента для исправления: расчёт MB неверный (лишнее деление), и DropOldest не считает реально потерянные сообщения (они дропаются молча внутри `push`, но метод возвращает `true`). Поправлю для честности данных.Теперь метрики честные. За 1.5 секунды unbounded-очередь выросла до **2 ГБ памяти** и продолжила бы расти — реальный OOM-сценарий при медленном клиенте и долгой сессии.

## Разбор результатов

**DisconnectConsumer** — самая показательная policy: `pushed=64` (ровно capacity), затем **немедленно** обнаружена проблема и producer остановлен — вместо бесконечных попыток впихнуть данные не успевающему клиенту, система явно признаёт "этот consumer не справляется" и прекращает работу с ним за один цикл переполнения.

**DropOldest** — `pushed=2 966 657`, но `overwritten=2 966 496` — то есть из почти 3 миллионов "успешных" push реально сохранилось только 64 (то, что осталось в очереди на момент остановки) плюс 97 уже потреблённых. Формально `push()` возвращал `true` почти всегда, но **99.98% сообщений были молча выброшены** сразу после записи — это архитектурно нормально для presence-обновлений ("печатает…", координаты курсора), где важно только последнее состояние, но было бы катастрофой для истории сообщений чата.

**DropNewest** — `dropped=3 534 030` явных потерь, `pushed=160` реально попало в очередь — новые сообщения теряются, пока в очереди остаётся место для более старых, что подходит, если порядок/полнота истории важнее полноты потока.

## Код (ключевые части)

```cpp
enum class DropPolicy { BlockRetry, DropNewest, DropOldest, DisconnectConsumer };

bool push(std::string v) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (data_.size() < Capacity) {
        data_.push_back(std::move(v));
        return true;
    }
    switch (policy_) {
        case DropPolicy::DropNewest:
            return false; // новое теряется, старое остаётся

        case DropPolicy::DropOldest:
            data_.pop_front();
            data_.push_back(std::move(v));
            overwritten_.fetch_add(1, std::memory_order_relaxed);
            return true; // новое принято, старое реально потеряно

        case DropPolicy::DisconnectConsumer:
            consumer_disconnected_.store(true, std::memory_order_release);
            return false; // явный сигнал наверх: клиент не успевает, отключить

        case DropPolicy::BlockRetry:
            return false; // обрабатывается в push_blocking снаружи
    }
    return false;
}

// Для BlockRetry: producer явно ждёт с таймаутом, а не молча теряет данные
bool push_blocking(std::string v, std::chrono::milliseconds timeout) {
    auto deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline) {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            if (data_.size() < Capacity) {
                data_.push_back(std::move(v));
                return true;
            }
        }
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    return false; // не дождались -- вызывающий код сам решает: разорвать соединение, залогировать, и т.д.
}
```

## Как выбирать policy для реального чата

|Тип данных|Рекомендуемая policy|Почему|
|---|---|---|
|Сообщения чата (текст, файлы)|**DisconnectConsumer** или `BlockRetry` с коротким таймаутом|Потеря сообщения — баг с точки зрения пользователя; лучше явно разорвать соединение и заставить клиента переподключиться с полной синхронизацией, чем тихо терять часть истории|
|Presence / "печатает…" / online-статус|**DropOldest**|Важно только последнее состояние; старое устарело в момент, когда пришло новое|
|Метрики / телеметрия|**DropNewest**|Не критично потерять недавние точки при всплеске нагрузки, важнее не тратить ресурсы на бесконечный retry|
|Критичные системные события (модерация, бан)|`BlockRetry` с достаточным таймаутом, либо отдельный **приоритетный** канал с большей capacity|Эти события не должны теряться вообще — иногда стоит завести для них отдельную структуру с более щедрым бюджетом памяти|

## Связь с MAX_RETRY из broadcast-демо

`MAX_RETRY` + drop в предыдущем `BroadcastChannel::publish()` — это фактически гибрид `BlockRetry` (ограниченный по числу попыток, не по времени) и `DropNewest` (после исчерпания попыток теряем именно новое сообщение для конкретного зависшего подписчика, а не всю рассылку). В реальном проде эту policy стоит параметризовать так же, как здесь — разным типам broadcast-сообщений (обычный текст vs presence) нужна разная стратегия.

**Полный код** (`backpressure_demo.cpp`) компилируется и запускается так же, как предыдущие демки:

```bash
g++ -std=c++20 -O2 -pthread backpressure_demo.cpp -o backpressure_demo
./backpressure_demo
```

Готовы перейти к последнему пункту Блока 3 — **reclamation problem** для broadcast-подписчиков (что происходит с памятью при disconnect, пока publisher ещё держит snapshot), или собрать всё в единый прототип чат-шины, как изначально планировали?

---
---

### **Reclamation проблема**: если пользователь отключается, когда безопасно освободить его структуры при lock-free доступе (hazard pointers / epoch-based / RCU)

Это ровно та reclamation-проблема, которую мы уже неявно решили в `broadcast_cow.cpp` через `shared_ptr` — но стоит явно показать **почему** это работает и **какой ценой**, а не просто "работает и ладно".Именно то, что нужно было доказать: `unsubscribe(1)` выполнился **между** взятием snapshot'а и его использованием, но "УДАЛЁН" для Subscriber 1 появляется **только после** строки "отправляю... подписчику 1" — то есть публикация безопасно отработала с уже "удалённым" (с точки зрения канала) подписчиком, и реальное освобождение памяти произошло ровно в момент выхода `snapshot` из scope, не раньше.

## Разбор — почему это работает и это НЕ hazard pointers, НЕ epoch-based

Каждый `shared_ptr<Subscriber>` внутри `SubscriberList` — это независимый refcount. Когда `publish()` берёт `snapshot = subscribers_.load()`, он получает `shared_ptr<const SubscriberList>` — refcount инкрементится **один раз** для всего вектора, а не по разу на каждого подписчика (вектор просто хранит копии `shared_ptr<Subscriber>`, которые были скопированы туда во время `subscribe()`/`unsubscribe()`, а не во время `publish()`).

Когда `unsubscribe(1)` строит новый вектор, он **не трогает** старый — старый вектор (тот, что держит `snapshot`) продолжает жить, пока у него есть хоть одна ссылка. `sub1`'s refcount остаётся ≥ 1, потому что старый вектор (внутри snapshot publisher'а) всё ещё хранит на него `shared_ptr`. Только когда `snapshot` разрушается на выходе из `publish_slow()` — refcount вектора падает до нуля → вектор разрушается → refcount каждого `shared_ptr<Subscriber>` внутри падает → если это была последняя ссылка на конкретного Subscriber, **тогда** вызывается его деструктор.

## Сравнение с hazard pointers и epoch-based — в чём принципиальная разница

||shared_ptr refcounting (наш случай)|Hazard Pointers|Epoch-based (RCU)|
|---|---|---|---|
|**Что отслеживается**|Число активных ссылок на **конкретный объект**|Явный список "кто сейчас держит какой указатель"|Глобальный счётчик "поколений", в какой эпохе кто находится|
|**Когда освобождается**|Точно в момент, когда счётчик достиг нуля — сразу, без задержки|Сразу, как только объект больше ни в одном hazard-слоте|Пакетно — когда все потоки "продвинулись" за эпоху удаления|
|**Стоимость на чтение**|Atomic inc/dec refcount при каждом **копировании** shared_ptr (здесь — не на каждый publish, а только при создании нового snapshot внутри subscribe/unsubscribe)|Atomic store hazard-слота + периодическая проверка scan|Atomic load эпохи при входе в критическую секцию|
|**Нужен ли явный "protected" список**|Нет — сам механизм refcounting уже это даёт|Да, отдельная инфраструктура|Да, отдельная инфраструктура|

**Ключевое отличие: `shared_ptr` даёт reclamation "из коробки" за счёт того, что каждый объект уже несёт свой счётчик** — не нужно городить отдельный per-thread hazard-реестр или эпохи, как в Treiber stack на raw pointers из более ранних разборов. Цена — тот же overhead, что разбирали для `atomic<shared_ptr<T>>`: atomic refcount operations при каждом копировании `shared_ptr`, и потенциальная нелок-фри реализация внутри самого `atomic<shared_ptr<...>>` на некоторых компиляторах (стоит проверить `is_lock_free()`, как обсуждали).

## Почему для broadcast-канала это оправданный выбор, а не для Treiber stack

**Частота операций разная.** В Treiber stack push/pop — это _hot path_, тысячи операций в секунду на структуру с миллионами узлов, там refcount-overhead на _каждую_ операцию заметен. В broadcast-канале refcount-инкремент происходит только при `subscribe`/`unsubscribe` (создание нового вектора с копиями shared_ptr) — **редко**, а сам `publish()` лишь читает уже существующие `shared_ptr` по ссылке (`const auto& sub`), не копируя и не трогая refcount вообще. Именно поэтому в комментариях к коду это было отмечено как "оправданное использование" — переиспользование дорогого механизма именно там, где он почти ничего не стоит.

## Практический вывод для собеседования

Если спросят "как вы решаете reclamation problem для списка подписчиков при disconnect" — правильный ответ не "используем hazard pointers всегда", а: **выбор механизма reclamation зависит от частоты read/write операций конкретной структуры.** Для hot-path lock-free структур (очереди, стеки с миллионами операций/сек) — hazard pointers или epoch-based, потому что там refcount-overhead был бы недопустим. Для структур с редкими мутациями и частыми, но дешёвыми чтениями (списки подписчиков, конфигурация, roster) — `atomic<shared_ptr<T>>` через COW часто оптимальнее по соотношению "простота кода / производительность", и вы уже получили reclamation бесплатно, без отдельной инфраструктуры.

Это закрывает последний пункт вашего плана Блока 3. Мы прошли весь путь от memory_order до полного прототипа чат-сервера (epoll-реактор → sharded воркеры → per-client SPSC → COW broadcast → backpressure → reclamation). Хотите собрать финальную сводку по всем трём блокам в виде документа/шпаргалки для подготовки к собеседованию, или есть конкретные темы, которые стоит повторить отдельно?

---
---

### **Практика:** собрать минимальный прототип — N потоков-писателей кладут сообщения в шину, M потоков-читателей (эмулируют коннекты) их разбирают, с метриками latency/throughput под нагрузкой.

Финальная практика — соберу самостоятельный прототип: N писателей кладут в общую bounded MPMC-шину, M читателей её разбирают, с полными метриками latency (p50/p99/p999) и throughput под нагрузкой.Все три сценария прошли без потерь (`dropped=0` везде), 8-9M msg/sec throughput, latency в диапазоне сотен микросекунд. Проверю на TSan для полной уверенности:TSan чист — все три сценария прошли без единого предупреждения о гонках. Вот полный код:

```cpp
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <algorithm>
#include <new>
#include <cassert>
#include <iomanip>

// ============================================================
// MPMC bus (Vyukov) -- итоговая версия из всех разборов:
// padding против false sharing, bounded с явным backpressure.
// ============================================================
template<typename T>
class MPMCBus {
    struct Cell { std::atomic<size_t> sequence; T data; };

    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> enqueue_pos_;
    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> dequeue_pos_;
    Cell* buffer_;
    size_t buffer_mask_;

public:
    explicit MPMCBus(size_t capacity)
        : buffer_(new Cell[capacity]), buffer_mask_(capacity - 1) {
        assert((capacity >= 2) && ((capacity & (capacity - 1)) == 0));
        for (size_t i = 0; i < capacity; ++i)
            buffer_[i].sequence.store(i, std::memory_order_relaxed);
        enqueue_pos_.store(0, std::memory_order_relaxed);
        dequeue_pos_.store(0, std::memory_order_relaxed);
    }
    ~MPMCBus() { delete[] buffer_; }
    MPMCBus(const MPMCBus&) = delete;

    bool try_push(T value) {
        Cell* cell; size_t pos = enqueue_pos_.load(std::memory_order_relaxed);
        for (;;) {
            cell = &buffer_[pos & buffer_mask_];
            size_t seq = cell->sequence.load(std::memory_order_acquire);
            intptr_t dif = (intptr_t)seq - (intptr_t)pos;
            if (dif == 0) {
                if (enqueue_pos_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) break;
            } else if (dif < 0) return false; // шина полна -- backpressure наверх
            else pos = enqueue_pos_.load(std::memory_order_relaxed);
        }
        cell->data = std::move(value);
        cell->sequence.store(pos + 1, std::memory_order_release);
        return true;
    }

    bool try_pop(T& result) {
        Cell* cell; size_t pos = dequeue_pos_.load(std::memory_order_relaxed);
        for (;;) {
            cell = &buffer_[pos & buffer_mask_];
            size_t seq = cell->sequence.load(std::memory_order_acquire);
            intptr_t dif = (intptr_t)seq - (intptr_t)(pos + 1);
            if (dif == 0) {
                if (dequeue_pos_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) break;
            } else if (dif < 0) return false; // шина пуста
            else pos = dequeue_pos_.load(std::memory_order_relaxed);
        }
        result = std::move(cell->data);
        cell->sequence.store(pos + buffer_mask_ + 1, std::memory_order_release);
        return true;
    }
};

// Сообщение несёт временную метку отправки -- нужна для честного
// измерения latency: время от push() ДО реального pop() (end-to-end).
struct Message {
    int producer_id;
    long long seq;
    std::chrono::steady_clock::time_point sent_at;
};

struct Metrics {
    std::vector<long long> latencies_ns;
    long long processed = 0;
};

void print_percentiles(const char* label, std::vector<long long>& lat) {
    if (lat.empty()) { std::cout << label << ": нет данных\n"; return; }
    std::sort(lat.begin(), lat.end());
    auto pct = [&](double p) {
        size_t idx = static_cast<size_t>(p * (lat.size() - 1));
        return lat[idx];
    };
    std::cout << label << " (n=" << lat.size() << "):\n"
              << "  p50:  " << std::setw(8) << pct(0.50) << " ns\n"
              << "  p90:  " << std::setw(8) << pct(0.90) << " ns\n"
              << "  p99:  " << std::setw(8) << pct(0.99) << " ns\n"
              << "  p999: " << std::setw(8) << pct(0.999) << " ns\n"
              << "  max:  " << std::setw(8) << lat.back() << " ns\n";
}

struct PrototypeResult {
    long long total_produced = 0;
    long long total_consumed = 0;
    long long total_dropped = 0;
    double wall_time_sec = 0;
    std::vector<long long> all_latencies_ns;
};

PrototypeResult run_prototype(int num_writers, int num_readers,
                                int messages_per_writer, size_t bus_capacity) {
    MPMCBus<Message> bus(bus_capacity);
    const long long total_target = static_cast<long long>(num_writers) * messages_per_writer;

    std::atomic<long long> produced{0}, consumed{0}, dropped{0};

    std::vector<Metrics> reader_metrics(num_readers);
    for (auto& m : reader_metrics) m.latencies_ns.reserve(messages_per_writer * num_writers / num_readers + 16);

    auto start = std::chrono::steady_clock::now();

    std::vector<std::thread> writers;
    for (int w = 0; w < num_writers; ++w) {
        writers.emplace_back([&, w] {
            for (int i = 0; i < messages_per_writer; ++i) {
                Message msg{w, i, std::chrono::steady_clock::now()};
                int attempts = 0;
                constexpr int MAX_ATTEMPTS = 10000;
                while (!bus.try_push(msg)) {
                    if (++attempts >= MAX_ATTEMPTS) {
                        dropped.fetch_add(1, std::memory_order_relaxed);
                        goto next_message;
                    }
                    std::this_thread::yield();
                }
                produced.fetch_add(1, std::memory_order_relaxed);
                next_message:;
            }
        });
    }

    std::vector<std::thread> readers;
    for (int r = 0; r < num_readers; ++r) {
        readers.emplace_back([&, r] {
            Message msg;
            auto& metrics = reader_metrics[r];
            while (consumed.load(std::memory_order_relaxed) + dropped.load(std::memory_order_relaxed) < total_target) {
                if (bus.try_pop(msg)) {
                    auto now = std::chrono::steady_clock::now();
                    auto latency_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now - msg.sent_at).count();
                    metrics.latencies_ns.push_back(latency_ns);
                    metrics.processed++;
                    consumed.fetch_add(1, std::memory_order_relaxed);
                } else {
                    std::this_thread::yield();
                }
            }
        });
    }

    for (auto& t : writers) t.join();
    for (auto& t : readers) t.join();

    auto end = std::chrono::steady_clock::now();

    PrototypeResult result;
    result.total_produced = produced.load();
    result.total_consumed = consumed.load();
    result.total_dropped = dropped.load();
    result.wall_time_sec = std::chrono::duration<double>(end - start).count();
    for (auto& m : reader_metrics) {
        result.all_latencies_ns.insert(result.all_latencies_ns.end(),
                                        m.latencies_ns.begin(), m.latencies_ns.end());
    }
    return result;
}

void print_result(const char* label, const PrototypeResult& r) {
    std::cout << "\n===== " << label << " =====\n";
    std::cout << "produced=" << r.total_produced << ", consumed=" << r.total_consumed
              << ", dropped=" << r.total_dropped << "\n";
    std::cout << "wall time: " << r.wall_time_sec << " s\n";
    std::cout << "throughput: " << static_cast<long long>(r.total_consumed / r.wall_time_sec) << " msg/sec\n";
    auto lat_copy = r.all_latencies_ns;
    print_percentiles("end-to-end latency", lat_copy);
}

int main() {
    constexpr size_t BUS_CAPACITY = 4096;
    constexpr int MESSAGES_PER_WRITER = 100'000;

    auto r1 = run_prototype(4, 4, MESSAGES_PER_WRITER, BUS_CAPACITY);
    print_result("4 writers / 4 readers", r1);

    auto r2 = run_prototype(8, 2, MESSAGES_PER_WRITER / 2, BUS_CAPACITY);
    print_result("8 writers / 2 readers (reader-bound)", r2);

    auto r3 = run_prototype(2, 8, MESSAGES_PER_WRITER, BUS_CAPACITY);
    print_result("2 writers / 8 readers (writer-bound)", r3);

    return 0;
}
```

**Сборка и запуск:**

```bash
g++ -std=c++20 -O2 -pthread final_prototype.cpp -o final_prototype
./final_prototype

# Обязательная проверка на гонки (уменьшите MESSAGES_PER_WRITER для скорости под TSan):
g++ -std=c++20 -O0 -g -fsanitize=thread -pthread final_prototype.cpp -o final_prototype_tsan
./final_prototype_tsan
```

## Что показывают три сценария

**4/4 симметричный** — baseline: 8.1M msg/sec, p50 245 мкс. p999 (833 мкс) заметно выше p50 — типичный длинный хвост из-за yield-based busy-wait вместо более умного backoff.

**8 writers / 2 readers (reader-bound)** — читатели становятся узким местом (2 потока разбирают вдвое больший поток писателей), но throughput **не упал**, а даже чуть вырос (9.3M msg/sec) — потому что per-slot contention MPMC-очереди хорошо масштабируется на запись даже при малом числе читателей. p99 здесь лучше (350 мкс vs 683 мкс) — меньше читателей означает меньше конкуренции за `dequeue_pos_`.

**2 writers / 8 readers (writer-bound)** — читатели простаивают (yield-loop), пока писателей мало — самая низкая latency на всех перцентилях (p999 всего 356 мкс) и самый чистый хвост, потому что как только сообщение появляется, кто-то из 8 читателей почти сразу его подхватывает.

## Важная методологическая оговорка

Этот контейнер даёт **1 видимый CPU** — реальной параллельности между потоками физически нет, только чередование планировщиком. Абсолютные цифры throughput здесь **не показательны** для настоящей многоядерной машины (там ожидается на порядок выше throughput при реальной параллельности, но и заметно более выраженная разница между reader-bound/writer-bound сценариями из-за реальной cache-line contention между ядрами, а не только на одном ядре). Обязательно перегоните это на своей рабочей машине — особенно интересно посмотреть на `perf c2c` при 8/8 конфигурации, чтобы увидеть реальный false-sharing/contention паттерн на `enqueue_pos_`/`dequeue_pos_`.

Этим прототипом закрывается вся практика Блока 3, а вместе с ней и весь план — от memory model до полной архитектуры чат-сервера с метриками под нагрузкой.

---
---
