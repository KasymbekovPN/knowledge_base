#include <boost/asio.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <iostream>
#include <format>
#include <string>

boost::asio::awaitable<void> send_request(const std::string host, const std::string message) {
    auto executor{co_await boost::asio::this_coro::executor};

    const std::string PORT{"5000"};
    boost::asio::ip::tcp::resolver resolver{executor};
    auto endpoints{co_await resolver.async_resolve(
        host,
        PORT,
        boost::asio::use_awaitable
    )};

    boost::asio::ip::tcp::socket socket{executor};
    co_await boost::asio::async_connect(
        socket,
        endpoints,
        boost::asio::use_awaitable
    );
    std::cout << std::format("Connected to {}:{}, send: {}\n", host, PORT, message);

    std::string request{message + "\n"};
    co_await boost::asio::async_write(
        socket,
        boost::asio::buffer(request),
        boost::asio::use_awaitable);

    char data[1024];
    std::size_t n{co_await socket.async_read_some(
        boost::asio::buffer(data),
        boost::asio::use_awaitable)};
    std::cout << std::format("Answer: {}\n", std::string(data, n));
}

int main(int argc, char *argv[]) {
    std::string host = (argc > 1) ? argv[1] : "consumer";
    std::string message = (argc > 2) ? argv[2] : "Привет от producer!";

    try {
        boost::asio::io_context io_context(1);
        boost::asio::co_spawn(io_context, send_request(host, message), boost::asio::detached);
        io_context.run();
    } catch (const std::exception& e) {
        std::cerr << std::format("Error: {}\n", e.what());
        return 1;
    }
    return 0;
}
