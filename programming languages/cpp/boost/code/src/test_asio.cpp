#include <boost/asio.hpp>
#include <chrono>
#include <iostream>
#include <format>
#include <memory>
#include <thread>
#include <array>

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

}
