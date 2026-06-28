#include <filesystem>
#include <boost/asio.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>

#include <iostream>
#include <format>
#include <thread>

namespace test_beast {

void test_sync_http_client() {
    const std::string HOST_NAME{"example.com"};
    const int HTTP11{11};
    try {
        boost::asio::io_context io;

        // 1. Резолвинг и подключение
        boost::asio::ip::tcp::resolver resolver{io};
        auto endpoints = resolver.resolve(HOST_NAME, "80");

        // Beast-обёртка над сокетом (+ таймауты)
        boost::beast::tcp_stream stream{io};
        stream.connect(endpoints);

        // 2. Формируем GET-запрос
        boost::beast::http::request<boost::beast::http::string_body> req{
            boost::beast::http::verb::get,
            "/",
            HTTP11
        };
        req.set(boost::beast::http::field::host, HOST_NAME);
        req.set(boost::beast::http::field::user_agent, "Beast-Client");

        // 3. Отправляем
        boost::beast::http::write(stream, req);

        // 4. Читаем ответ
        boost::beast::flat_buffer buffer;
        boost::beast::http::response<boost::beast::http::string_body> res;
        boost::beast::http::read(stream, buffer, res);


        // 5. Выводим
        const int BODY_MAX{200};
        std::cout
            << std::format("Status: {}\n", res.result_int())
            << std::format("Content-Type: {}\n", res[boost::beast::http::field::content_type])
            << std::format("Body (first {}):\n{}\n", BODY_MAX, res.body().substr(0, BODY_MAX));

        // 6. Закрываем
        boost::beast::error_code ec;
        stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    } catch (const std::exception& e) {
        std::cerr << std::format("Error: {}\n", e.what());
    }
}

void handle_sync_http_server_connection(boost::asio::ip::tcp::socket _socket) {
    try {
        boost::beast::flat_buffer buffer;

        for (;;) {
            // читаем запрос
            boost::beast::http::request<boost::beast::http::string_body> req;
            boost::beast::error_code ec;
            boost::beast::http::read(_socket, buffer, req, ec);

            // клиент закрыл
            if (ec == boost::beast::http::error::end_of_stream) break;
            if (ec) throw boost::beast::system_error{ec};

            // простейший «роутинг» по target
            boost::beast::http::response<boost::beast::http::string_body> res{
                boost::beast::http::status::ok,
                req.version()
            };
            res.set(boost::beast::http::field::server, "Beast");
            res.set(boost::beast::http::field::content_type, "text/plain; charset=utf-8");

            if (req.target() == "/") { res.body() = "Main page\n"; }
            else if (req.target() == "/hello") { res.body() = "Hello, world!\n"; }
            else {
                res.result(boost::beast::http::status::not_found);
                res.body() = "404 Not Found\n";
            }

            res.keep_alive(req.keep_alive());
            res.prepare_payload();
            boost::beast::http::write(_socket, res);

            if (!req.keep_alive()) break;
        }

        boost::beast::error_code ec;
        _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
    } catch (const std::exception& e) {
        std::cerr << std::format("Session error: {}\n", e.what());
    }
}

// curl.exe http://localhost:9000
// curl.exe http://localhost:9000/hello
void test_sync_http_server() {
    const int PORT{9000};
    try {
        boost::asio::io_context io;
        boost::asio::ip::tcp::acceptor acceptor{
            io,
            boost::asio::ip::tcp::endpoint{
                boost::asio::ip::tcp::v4(),
                PORT
            }
        };
        std::cout << std::format("HTTP-server on http://localhost:{}\n", PORT);

        for (;;) {
            boost::asio::ip::tcp::socket socket{io};
            // ждём соединение
            acceptor.accept(socket);

            // каждое соединение — в отдельном потоке (упрощённо)
            std::thread{handle_sync_http_server_connection, std::move(socket)}.detach();
        }
    } catch (const std::exception& e) {
        std::cerr << std::format("Error: {}\n", e.what());
    }
}

static boost::asio::awaitable<void> handle_session(boost::asio::ip::tcp::socket _socket) {
    boost::beast::flat_buffer buffer;
    try {
        for (;;) {
            boost::beast::http::request<boost::beast::http::string_body> req;
            co_await boost::beast::http::async_read(_socket, buffer, req, boost::asio::use_awaitable);

            boost::beast::http::response<boost::beast::http::string_body> res{
                boost::beast::http::status::ok,
                req.version()
            };
            res.set(boost::beast::http::field::server, "Beast-Async");
            res.set(boost::beast::http::field::content_type, "text/plain");
            res.body() = std::format("Answer on {}\n", std::string{req.target()});
            res.keep_alive(req.keep_alive());
            res.prepare_payload();

            co_await boost::beast::http::async_write(_socket, res, boost::asio::use_awaitable);
            if (!req.keep_alive()) break;
        }
    } catch (const std::exception& e) {
        std::cerr << std::format("Error: {}\n", e.what());
    }

    boost::beast::error_code ec;
    _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
}

static boost::asio::awaitable<void> listener(
    boost::asio::io_context& _io,
    unsigned short _port) {

    boost::asio::ip::tcp::acceptor acceptor{_io, {boost::asio::ip::tcp::v4(), _port}};
    std::cout << std::format("HTTP-server on port: {}\n", _port);
    for (;;) {
        boost::asio::ip::tcp::socket socket = co_await acceptor.async_accept(
            boost::asio::use_awaitable);
        boost::asio::co_spawn(_io, handle_session(std::move(socket)), boost::asio::detached);
    }
}

void test_async_http_server() {
    boost::asio::io_context io;
    boost::asio::co_spawn(io, listener(io, 9000), boost::asio::detached);
    io.run();
}

void start_ws_client() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    const std::string HOST{"localhost"};
    const std::string PORT{"9001"};
    try {
        boost::asio::io_context io;

        // резолвинг и подключение
        boost::asio::ip::tcp::resolver resolver{io};
        auto const endpoints = resolver.resolve(HOST, PORT);

        boost::beast::websocket::stream<boost::asio::ip::tcp::socket> ws{io};
        auto ep = boost::asio::connect(ws.next_layer(), endpoints);

        // host для заголовка должен включать порт
        std::string host_header = std::format("{}:{}", HOST, ep.port());

        // клиентское рукопожатие
        ws.handshake(host_header, "/");
        std::cout << "Connecting to WS-server\n";

        // отправляем сообщение
        ws.write(boost::asio::buffer(std::string{"Hello, WS-server"}));

        // читаем эхо-ответ
        boost::beast::flat_buffer buffer;
        ws.read(buffer);
        std::cout << "Answer: " << boost::beast::make_printable(buffer.data()) << "\n";

        // корректно закрываем
        ws.close(boost::beast::websocket::close_code::normal);
        std::cout << "Connection closed\n";
    } catch (const std::exception& e) {
        std::cerr << std::format("Error: {}\n", e.what());
    }
}

void do_ws_session(boost::asio::ip::tcp::socket _socket) {
    try {
        boost::beast::websocket::stream<boost::asio::ip::tcp::socket> ws{std::move(_socket)};

        // серверное рукопожатие (превращает HTTP-апгрейд в WebSocket)
        ws.accept();
        std::cout << "WS-connection established\n";

        for (;;) {
            boost::beast::flat_buffer buffer;
            // читаем сообщение
            ws.read(buffer);

            std::cout << "Read: " << boost::beast::make_printable(buffer.data()) << "\n";
            // отвечаем тем же типом
            ws.text(ws.got_text());
            // эхо обратно
            ws.write(buffer.data());
        }
    } catch (const boost::beast::system_error& se) {
        if (se.code() != boost::beast::websocket::error::closed) {
            std::cerr << std::format("Error: {}\n", se.code().message());
        } else {
            std::cout << std::format("Client closed connection\n");
        }
    } catch (const std::exception& e) {
        std::cerr << std::format("Error: {}\n", e.what());
    }
}

void test_ws() {
    std::thread t0{start_ws_client};
    std::thread t1{[]() {
        try {
            boost::asio::io_context io;
            boost::asio::ip::tcp::acceptor acceptor{
                io,
                boost::asio::ip::tcp::endpoint{
                    boost::asio::ip::tcp::v4(),
                    9001
                }
            };
            std::cout << "WebSocket-server on port: ws://localhost:9001\n";

            for (;;) {
                boost::asio::ip::tcp::socket socket{io};
                acceptor.accept(socket);
                std::thread{do_ws_session, std::move(socket)}.detach();
            }
        } catch (const std::exception& e) {
            std::cerr << std::format("Error: {}\n", e.what());
        }
    }};

    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    t0.join();
    t1.join();
}

}
