// Идея: io_context - это и есть event loop/reactor (на Linux под капотом epoll).
// Все async_* операции только РЕГИСТРИРУЮТ обработчик и сразу возвращают управление.
// Реальная диспетчеризация происходит внутри io.run() - единственного блокирующего
// вызова в программе. Именно там поток спит в epoll_wait и просыпается либо на
// готовый сокет, либо на истёкший таймер (см. предыдущий разбор: таймаут ожидания
// в reactor'е равен времени до ближайшего таймера).

#include <boost/asio.hpp>
#include <csignal>
#include <array>
#include <chrono>
#include <iostream>
#include <format>
#include <memory>
#include <string>

// ---------------------------------------------------------------------------
// EchoSession: одно клиентское соединение.
// Живёт, пока жив хотя бы один pending async-вызов, который держит shared_ptr
// на себя (shared_from_this). Как только оба (read/write) handler'а отработали
// с ошибкой (например eof) и не переставили следующую операцию - последний
// shared_ptr выходит из scope, сессия уничтожается сама.
// ---------------------------------------------------------------------------
class EchoSession: public std::enable_shared_from_this<EchoSession> {
public:
    explicit EchoSession(boost::asio::ip::tcp::socket socket):
        socket_{std::move(socket)} {}
    void start() {
        std::cout << "[session] new connection\n";
        do_read();
    }
private:
    void do_read() {
        auto self = shared_from_this();
        socket_.async_read_some(
            boost::asio::buffer(buffer_),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (ec) {
                    if (ec == boost::asio::error::eof) {
                        std::cout << "[session] client turned off\n";
                    } else {
                        std::cerr << std::format("[session] read error: {}\n", ec.message());
                    }
                    // не переставляем doRead() -> сессия умрёт
                    return;
                }
                std::cout << std::format("[session] received {} bites, echo handled\n", length);
                do_write(length);
            });
    }

    void do_write(std::size_t length) {
        auto self = shared_from_this();
        boost::asio::async_write(
            socket_,
            boost::asio::buffer(buffer_, length),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (ec) {
                    std::cerr << std::format("[session] wrire error: {}\n", ec.message());
                    return;
                }
                // снова регистрируем ожидание данных, не блокируясь
                do_read();
            });
    }

    boost::asio::ip::tcp::socket socket_;
    std::array<char, 1024> buffer_{};
};

// ---------------------------------------------------------------------------
// Server: слушающий сокет, принимает подключения асинхронно.
// ---------------------------------------------------------------------------
class Server {
public:
    explicit Server(boost::asio::io_context &io, unsigned short port):
        acceptor_{io, boost::asio::ip::tcp::endpoint{boost::asio::ip::tcp::v4(), port}} {
        do_accept();
    }
private:
    void do_accept() {
        acceptor_.async_accept([this](
            boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
            if (ec) {
                std::cerr << std::format("[server] accept error: {}\n", ec.message());
            } else {
                std::make_shared<EchoSession>(std::move(socket))->start();
            }
            // регистрируем ожидание следующего клиента
            do_accept();
        });
    }

    boost::asio::ip::tcp::acceptor acceptor_;
};

// ---------------------------------------------------------------------------
// PeriodicTimer: таймер, который сам себя переставляет (re-arm) после каждого
// срабатывания - отсюда периодичность. Один-единственный экземпляр steady_timer
// на весь жизненный цикл, никаких потоков не создаётся.
// ---------------------------------------------------------------------------
class PeriodicTimer {
public:
    explicit PeriodicTimer(boost::asio::io_context &io,
                           const std::chrono::milliseconds interval,
                           std::string name):
        timer_{io},
        interval_{interval},
        name_{std::move(name)} {
        schedule_next();
    }

private:
    void schedule_next() {
        timer_.expires_after(interval_);
        timer_.async_wait([this](boost::system::error_code ec) {
            // таймер отменён (например при остановке io_context)
            if (ec) return;
            std::cout << std::format("timer: {} tick\n", name_);
            schedule_next();
        });
    }

    boost::asio::steady_timer timer_;
    std::chrono::milliseconds interval_;
    std::string name_;
};

// & "C:\Program Files (x86)\Nmap\ncat.exe" localhost 12345
int main() {
    boost::asio::io_context io;

    Server server{io, 12345};
    PeriodicTimer heartbeat{io, std::chrono::seconds{2}, "heartbeat"};
    PeriodicTimer stats{io, std::chrono::seconds{5}, "stats"};

    // Graceful shutdown по Ctrl+C: тоже событие в том же loop'е, а не отдельный поток.
    boost::asio::signal_set signals(io, SIGINT, SIGTERM);
    signals.async_wait([&io](boost::system::error_code ec, int) {
        std::cout << "\nReceived stop signal, stopping...\n";
        io.stop();
    });

    std::cout << "The server is listening on port 12345. A single threaded event loop is running.\n";
    std::cout << "Connect: nc localhost 12345 (linux)\n";
    std::cout << "Connect: ncat localhost 12345 (windows)\n";

    // Единственный блокирующий вызов во всей программе. Внутри - reactor
    // (epoll_wait на Linux), который спит до ближайшего события: либо сокет
    // готов, либо истёк таймер, либо пришёл сигнал. Все три источника событий
    // (accept/read/write, таймеры, сигналы) мультиплексируются в одном потоке.
    io.run();

    return 0;
}
