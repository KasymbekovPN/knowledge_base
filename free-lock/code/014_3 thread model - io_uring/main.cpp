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
