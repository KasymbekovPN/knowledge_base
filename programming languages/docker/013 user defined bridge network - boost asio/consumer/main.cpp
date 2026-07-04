#include <boost/asio.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <iostream>
#include <format>
#include <future>
#include <string>

// Обрабатывает одного клиента как отдельную корутину.
// Несколько клиентов могут обслуживаться конкурентно на одном потоке io_context.
boost::asio::awaitable<void> handle_client(boost::asio::ip::tcp::socket socket) {
    try {
        for (;;) {
            char data[1024];
            std::size_t n{co_await socket.async_read_some(
                boost::asio::buffer(data),
                boost::asio::use_awaitable
            )};
            std::string received{data, n};
            while (!received.empty()
                && (received.back() == '\n' || received.back() == '\r')) {
                received.pop_back();
            }
            std::cout << std::format("Received: {}\n", received);

            std::string response{std::format("ECHO: {}\n", received)};
            co_await boost::asio::async_write(
                socket,
                boost::asio::buffer(response),
                boost::asio::use_awaitable
            );
        }
    } catch (const std::exception& e) {
        // Обрыв соединения клиентом бросает исключение из async_read_some — это нормальный сценарий
        std::cout << std::format("Client turned off: {}\n", e.what());
    }
}

boost::asio::awaitable<void> listener() {
    const int PORT{5000};

    auto executor = co_await boost::asio::this_coro::executor;
    boost::asio::ip::tcp::acceptor acceptor(
        executor,
        boost::asio::ip::tcp::endpoint(
            boost::asio::ip::tcp::v4(),
            PORT
        )
    );
    std::cout << std::format("Consumer listen to port {}\n", PORT);
    for (;;) {
        boost::asio::ip::tcp::socket socket = co_await acceptor.async_accept(
            boost::asio::use_awaitable
        );
        // Каждый новый клиент — отдельная независимая корутина
        boost::asio::co_spawn(
            executor,
            handle_client(std::move(socket)),
            boost::asio::detached
        );
    }
}

int main() {
    try {
        boost::asio::io_context io_context(1);
        boost::asio::co_spawn(io_context, listener(), boost::asio::detached);
        io_context.run();
    } catch (const std::exception& e) {
        std::cerr << std::format("Exception: {}\n", e.what());
        return 1;
    }

    return 0;
}
