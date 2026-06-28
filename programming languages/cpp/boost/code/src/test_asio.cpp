#include <boost/asio.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <chrono>
#include <iostream>
#include <format>
#include <memory>
#include <thread>
#include <array>

#include "../../../IO/code/socket/socketbuf.h"
#include "../.build/vcpkg_installed/x64-windows/share/boost-1.91.0/random/haertel.hpp"

namespace test_asio {

void test_timer() {
    const int SECONDS{2};
    boost::asio::io_context io;
    boost::asio::steady_timer timer{io, std::chrono::seconds{SECONDS}};

    timer.async_wait([seconds = SECONDS](const boost::system::error_code& ec) {
        if (!ec) std::cout << std::format("The timer went off after {} seconds!\n", seconds);
    });

    std::cout << "Wait...\n";
    io.run();
}

void test_buffer() {
    std::string message{"Hello"};
    auto buf = boost::asio::buffer(message);

    std::cout << "Buffer size: " << buf.size() << "\n";
    std::cout << "Data: " << std::string(static_cast<const char*>(buf.data()), buf.size()) << "\n";
}

void test_sync_tcp() {
    const std::string HOST_NAME{"example.com"};
    boost::asio::io_context io;
    boost::asio::ip::tcp::resolver resolver{io};
    auto endpoints = resolver.resolve(HOST_NAME, "80");

    boost::asio::ip::tcp::socket socket {io};
    boost::asio::connect(socket, endpoints);

    std::string request =
         std::format("GET / HTTP/1.1\r\nHost: {}\r\nConnection: close\r\n\r\n", HOST_NAME);
    boost::asio::write(socket, boost::asio::buffer(request));

    boost::asio::streambuf response;
    boost::system::error_code ec;
    boost::asio::read(socket, response, ec);

    std::cout
        << "EC: " << ec.message() << "\n"
        << "Response:\n" << &response << std::endl;
}

// --- server part (async) ---

// Одно соединение; shared_ptr держит объект живым на время async-операций
class Session: public std::enable_shared_from_this<Session> {
public:
    explicit Session(boost::asio::ip::tcp::socket _socket):
        socket_(std::move(_socket)) {}

    void start() { read(); }
private:
    void read() {
        auto self = shared_from_this();
        socket_.async_read_some(
            boost::asio::buffer(data_),
            [this, self] (boost::system::error_code _ec, std::size_t _length) {
                if (!_ec) {
                    std::cout << std::format("[SERVER] Received: {}\n", std::string(data_.data(), _length));
                    write(_length);
                } else {
                    std::cout << std::format("[SERVER] Connection closed: {}\n", _ec.message());
                }
            }
        );
    }

    void write(std::size_t _length) {
        auto self = shared_from_this();
        boost::asio::async_write(
            socket_,
            boost::asio::buffer(data_, _length),
            [this, self](boost::system::error_code _ec, std::size_t) {
                if (!_ec) read();
            }
        );
    }

    boost::asio::ip::tcp::socket socket_;
    std::array<char, 1024> data_;
};

class Server {
public:
    Server(boost::asio::io_context& _io, unsigned short _port):
        acceptor_{_io, boost::asio::ip::tcp::endpoint{boost::asio::ip::tcp::v4(), _port}} {
        accept();
    }
private:
    void accept() {
        acceptor_.async_accept(
            [this](boost::system::error_code _ec, boost::asio::ip::tcp::socket _socket) {
                if (!_ec) {
                    std::cout << "[SERVER] New connection\n";
                    std::make_shared<Session>(std::move(_socket))->start();
                }
                accept();
            }
        );
    }
    boost::asio::ip::tcp::acceptor acceptor_;
};

// --- client part(sync) ---

void start_client_thread() {
    try {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));

        boost::asio::io_context client_io;
        boost::asio::ip::tcp::socket socket{client_io};

        boost::asio::ip::tcp::resolver resolver{client_io};
        auto endpoints = resolver.resolve("127.0.0.1", "12345");
        boost::asio::connect(socket, endpoints);
        std::cout << "[CLIENT] Connected to server\n";

        for (int i{}; i <= 5; ++i) {
            std::string message = std::format("Hello {}\n", i);
            boost::asio::write(socket, boost::asio::buffer(message));
            std::cout << std::format("[CLIENT] Send: {}\n", message);

            std::array<char, 1024> replay;
            std::size_t n{socket.read_some(boost::asio::buffer(replay))};
            std::cout << std::format("[CLIENT] Echo: {}\n", std::string(replay.data(), n));
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        boost::system::error_code ec;
        socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        socket.close(ec);
        std::cout << std::format("[CLIENT] Shutting down...\n");
    } catch (const std::exception& e) {
        std::cout << std::format("[CLIENT] Exception: {}\n", e.what());
    }
}

void test_async_tcp() {
    const unsigned short PORT{12345};

    boost::asio::io_context io;
    Server server{io, PORT};
    std::cout << std::format("Server is listening to port {}\n", PORT);

    std::thread client_thread{start_client_thread};
    std::thread stop_thread{[&io]() {
        std::this_thread::sleep_for(std::chrono::seconds(7));
        io.stop();
    }};

    io.run();

    client_thread.join();
    stop_thread.join();

    std::cout << "DONE!\n";
}

// Общий ресурс, к которому обращаются обработчики из разных потоков
class Counter: public std::enable_shared_from_this<Counter> {
public:
    Counter(
        boost::asio::io_context& _io,
        int _id,
        std::chrono::milliseconds _interval = std::chrono::milliseconds(10)
        ):
        strand_{boost::asio::make_strand(_io)},
        timer_{_io},
        id_{_id},
        interval_{_interval} {}

    void start() { schedule(); }
    unsigned long long value() const { return value_; }
private:
    void schedule() {
        timer_.expires_after(interval_);

        auto self = shared_from_this();
        // bind_executor связывает обработчик со strand'ом:
        // обработчики этого strand'а НЕ выполнятся одновременно
        timer_.async_wait(boost::asio::bind_executor(
            strand_,
            [this, self](const boost::system::error_code& _ec) {
                if (!_ec) on_tick();
            }));
    }

    void on_tick() {
        // Этот код защищён strand'ом: даже при 4 потоках в пуле
        // два on_tick одного Counter НЕ выполнятся параллельно.
        // Поэтому ++value_ безопасен БЕЗ мьютекса.

        if (++value_ < 100) { schedule(); }
    }

    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    boost::asio::steady_timer timer_;
    int id_{0};
    unsigned long long value_{0};
    std::chrono::milliseconds interval_;
};


void test_strands_for_counter() {
    boost::asio::io_context io;

    // удерживаем io_context от преждевременной остановки
    auto work = boost::asio::make_work_guard(io);

    // создаём несколько счётчиков, каждый со своим strand'ом
    std::vector<std::shared_ptr<Counter>> counters;
    for (int i{}; i < 3; ++i) {
        auto c = std::make_shared<Counter>(io, i, std::chrono::milliseconds(100));
        c->start();
        counters.push_back(c);
    }

    // пул из 4 потоков — обработчики могут идти параллельно
    std::vector<std::thread> pool;
    for (int i{}; i < 4; ++i)
        pool.emplace_back([&io] { io.run(); });

    // даём поработать и останавливаем
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    // разрешаем io.run() завершиться, когда работа кончится
    work.reset();
    io.stop();

    for (auto& t : pool) t.join();

    for (auto& c : counters) {
        std::cout << std::format("Final value: {}\n", c->value());
    }
}

// --- SERVER ---
static boost::asio::awaitable<void> handle_session(boost::asio::ip::tcp::socket _socket) {
    try {
        char data[1024];
        for (;;) {
            // co_await приостанавливает корутину, не блокируя поток.
            // Когда данные придут, выполнение продолжится с этой строки.
            std::size_t n{co_await _socket.async_read_some(
                boost::asio::buffer(data),
                boost::asio::use_awaitable)};

            std::cout << std::format("[SERVER] Took: {}\n", std::string(data, n));

            // отправляем эхо обратно — снова co_await
            co_await boost::asio::async_write(
                _socket,
                boost::asio::buffer(data, n),
                boost::asio::use_awaitable);
        }
    } catch (const std::exception& e) {
        // при закрытии соединения async_read_some бросит исключение (EOF)
        std::cout << std::format("[SERVER] Session closed: {}\n", e.what());
    }
}

// --- SERVER ---

static boost::asio::awaitable<void> listener(boost::asio::io_context& _io, unsigned short _port) {
    boost::asio::ip::tcp::acceptor acceptor(
        _io,
        boost::asio::ip::tcp::endpoint(
            boost::asio::ip::tcp::v4(),
            _port
        )
    );
    std::cout << std::format("[SERVER] listen to port {}\n", _port);

    for (;;) {
        // ждём входящее соединение асинхронно
        boost::asio::ip::tcp::socket socket = co_await acceptor.async_accept(
            boost::asio::use_awaitable
        );
        std::cout << std::format("[SERVER] New connection\n");

        // запускаем обработку сессии как отдельную корутину;
        // detached — нам не нужен её результат, она живёт сама по себе
        boost::asio::co_spawn(_io, handle_session(std::move(socket)), boost::asio::detached);
    }
}

// --- CLIENT ---
boost::asio::awaitable<void> client(boost::asio::io_context& _io) {
    try {
        boost::asio::ip::tcp::resolver resolver(_io);
        // асинхронный DNS-резолвинг
        auto endpoints = co_await resolver.async_resolve(
            "127.0.0.1",
            "12345",
            boost::asio::use_awaitable);
        boost::asio::ip::tcp::socket socket{_io};
        // асинхронное подключение
        co_await boost::asio::async_connect(socket, endpoints, boost::asio::use_awaitable);
        std::cout << std::format("[CLIENT] Connected\n");

        for (int i{1}; i <= 5; ++i) {
            std::string message{std::format("Message #{}\n", i)};

            // отправка
            co_await boost::asio::async_write(
                socket,
                boost::asio::buffer(message),
                boost::asio::use_awaitable);
            std::cout << std::format("[CLIENT] Send: {}\n", message);

            // чтение эха
            char reply[1024];
            std::size_t n{co_await socket.async_read_some(
                boost::asio::buffer(reply),
                boost::asio::use_awaitable)};
            std::cout << std::format("[CLIENT] Echo: {}\n", std::string(reply, n));

            // асинхронная пауза через таймер (не блокирует поток!)
            boost::asio::steady_timer timer{_io, std::chrono::milliseconds(1000)};
            co_await timer.async_wait(boost::asio::use_awaitable);
        }

        boost::system::error_code ec;
        socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        std::cout << std::format("[CLIENT] Shutting down...\n");
    } catch (const std::exception& e) {
        std::cerr << std::format("[CLIENT] Exception: {}\n", e.what());
    }
}

void test_coroutine() {
    try {
        boost::asio::io_context io;

        // запускаем серверную корутину
        boost::asio::co_spawn(io, listener(io, 12345), boost::asio::detached);

        // запускаем клиентскую корутину с небольшой задержкой
        boost::asio::co_spawn(io, [&io]() -> boost::asio::awaitable<void> {
            boost::asio::steady_timer t{io, std::chrono::milliseconds(300)};
            co_await t.async_wait(boost::asio::use_awaitable);
            co_await client(io);

            // клиент закончил — останавливаем io_context через 1 секунду
            boost::asio::steady_timer t2{io, std::chrono::milliseconds(1000)};
            co_await t2.async_wait(boost::asio::use_awaitable);
            io.stop();
        }, boost::asio::detached);

        // один поток крутит всё: и сервер, и клиент
        io.run();

        std::cout << std::format("[CLIENT] Done!\n");
    } catch (const std::exception& e) {
        std::cerr << std::format("[CLIENT] Exception: {}\n", e.what());
    }
}

}
