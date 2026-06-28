---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/boost/_|<=]]

# Boost.Beast

Boost.Beast — библиотека для HTTP и WebSocket поверх Boost.Asio. Header-only, но опирается на весь механизм Asio (`io_context`, сокеты, async-модель, корутины). Изучать **только после уверенного владения Asio** — Beast не заменяет его, а надстраивает протокольный слой над его сокетами.

```cpp
#include <boost/beast.hpp>
namespace beast = boost::beast;
namespace http  = beast::http;
namespace websocket = beast::websocket;
namespace net   = boost::asio;
using tcp = net::ip::tcp;
```

CMake:

```cmake
find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)
target_link_libraries(app PRIVATE Boost::system Threads::Threads)
# для HTTPS/WSS дополнительно понадобится OpenSSL
```

## Что Beast даёт и чего не даёт

**Даёт:** типы и алгоритмы для разбора/формирования HTTP-сообщений и WebSocket-фреймов, поверх Asio-стримов.

**Не даёт:** это **низкоуровневая** библиотека. Здесь нет роутинга, middleware, готового веб-фреймворка. Ты вручную читаешь запрос, смотришь метод/путь, формируешь ответ. Для полноценного веб-сервера поверх Beast часто берут надстройки (например, Crow, Drogon используют свои стеки; поверх Beast строят собственные роутеры).

## Часть 1. HTTP

### 1.1 Типы сообщений

|Тип|Описание|
|---|---|
|`http::request<Body>`|HTTP-запрос|
|`http::response<Body>`|HTTP-ответ|
|`http::message<isRequest, Body, Fields>`|Обобщённое сообщение (база)|

### Body-типы (как хранится тело сообщения)

|Тип|Применение|
|---|---|
|`http::string_body`|Тело как `std::string` — самый частый|
|`http::dynamic_body`|Тело в динамическом буфере|
|`http::empty_body`|Без тела|
|`http::file_body`|Тело из файла (отдача статики)|
|`http::buffer_body`|Потоковая передача по кускам|

### 1.2 Структура запроса

```cpp
http::request<http::string_body> req{http::verb::get, "/path", 11};
//                                    ^метод          ^target ^HTTP/1.1
req.set(http::field::host, "example.com");
req.set(http::field::user_agent, "Beast");
req.body() = "тело запроса";   // для POST/PUT
req.prepare_payload();          // выставить Content-Length и др.
```

| Элемент              | Описание                                                          |
| -------------------- | ----------------------------------------------------------------- |
| `http::verb`         | Метод: `get`, `post`, `put`, `delete_`, `head`, ...               |
| `.target()`          | Путь запроса (`/api/users`)                                       |
| `.version()`         | Версия: `11` = HTTP/1.1, `10` = HTTP/1.0                          |
| `.set(field, value)` | Установить заголовок                                              |
| `http::field`        | Перечисление стандартных заголовков (`host`, `content_type`, ...) |
| `.body()`            | Тело сообщения                                                    |
| `.prepare_payload()` | Согласовать Content-Length/Transfer-Encoding                      |

### 1.3 Структура ответа

```cpp
http::response<http::string_body> res{http::status::ok, 11};
res.set(http::field::server, "Beast");
res.set(http::field::content_type, "text/html");
res.body() = "<h1>Привет</h1>";
res.prepare_payload();
```

|Элемент|Описание|
|---|---|
|`http::status`|Код статуса: `ok`(200), `not_found`(404), `internal_server_error`(500)...|
|`.result()`|Получить/установить статус|
|`.reason()`|Текстовая причина статуса|

### 1.4 Операции чтения/записи

|Sync|Async|Описание|
|---|---|---|
|`http::read(stream, buffer, msg)`|`http::async_read(...)`|Прочитать сообщение целиком|
|`http::write(stream, msg)`|`http::async_write(...)`|Записать сообщение|
|`http::read_header(...)`|`http::async_read_header(...)`|Прочитать только заголовки|

Для чтения нужен буфер `beast::flat_buffer` (накапливает байты между чтениями).

## 1.5 Полный пример: синхронный HTTP-клиент

### src/test_beast.cpp
```cpp
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <iostream>

// ...

#include <boost/asio.hpp>  
#include <boost/beast.hpp>  
  
#include <iostream>  
#include <format>  
  
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
    }}  
  
}
```

```
Status: 200
Content-Type: text/html
Body (first 200):
<!doctype html><html lang="en"><head><title>Example Domain</title><link rel="icon" href="data:,"><meta name="viewport" content="width=device-width, initial-scale=1"><style>body{background:#eee;width:6
```

## 1.6 Полный пример: синхронный HTTP-сервер

### src/test_beast.cpp
```cpp
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <thread>

// ...

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
            };            res.set(boost::beast::http::field::server, "Beast");  
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
    }}  
  
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
        };        std::cout << std::format("HTTP-server on http://localhost:{}\n", PORT);  
  
        for (;;) {  
            boost::asio::ip::tcp::socket socket{io};  
            // ждём соединение  
            acceptor.accept(socket);  
  
            // каждое соединение — в отдельном потоке (упрощённо)  
            std::thread{handle_sync_http_server_connection, std::move(socket)}.detach();  
        }    } catch (const std::exception& e) {  
        std::cerr << std::format("Error: {}\n", e.what());  
    }}
```

> Это упрощённый сервер (поток на соединение). Для продакшена используют асинхронную модель с пулом потоков и strand'ами — пример ниже.

## 1.7 Асинхронный HTTP-сервер на корутинах (C++20)
### src/test_beast.cpp
```cpp
static boost::asio::awaitable<void> handle_session(boost::asio::ip::tcp::socket _socket) {  
    boost::beast::flat_buffer buffer;  
    try {  
        for (;;) {  
            boost::beast::http::request<boost::beast::http::string_body> req;  
            co_await boost::beast::http::async_read(_socket, buffer, req, boost::asio::use_awaitable);  
  
            boost::beast::http::response<boost::beast::http::string_body> res{  
                boost::beast::http::status::ok,  
                req.version()  
            };            res.set(boost::beast::http::field::server, "Beast-Async");  
            res.set(boost::beast::http::field::content_type, "text/plain");  
            res.body() = std::format("Answer on {}\n", std::string{req.target()});  
            res.keep_alive(req.keep_alive());  
            res.prepare_payload();  
  
            co_await boost::beast::http::async_write(_socket, res, boost::asio::use_awaitable);  
            if (!req.keep_alive()) break;  
        }    } catch (const std::exception& e) {  
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
    }}  
  
void test_async_http_server() {  
    boost::asio::io_context io;  
    boost::asio::co_spawn(io, listener(io, 9000), boost::asio::detached);  
    io.run();  
}
```

## Часть 2. WebSocket

WebSocket даёт постоянное двунаправленное соединение поверх одного TCP. Beast реализует рукопожатие (handshake) и фрейминг.

### 2.1 Ключевой тип

```cpp
websocket::stream<tcp::socket>           // WebSocket поверх TCP
websocket::stream<beast::tcp_stream>     // с таймаутами Beast
websocket::stream<ssl::stream<tcp::socket>> // WSS (защищённый)
```

### 2.2 Основные операции

|Sync|Async|Описание|
|---|---|---|
|`ws.handshake(host, target)`|`ws.async_handshake(...)`|Клиентское рукопожатие|
|`ws.accept()`|`ws.async_accept(...)`|Серверное рукопожатие|
|`ws.read(buffer)`|`ws.async_read(...)`|Прочитать сообщение|
|`ws.write(buffer)`|`ws.async_write(...)`|Отправить сообщение|
|`ws.close(reason)`|`ws.async_close(...)`|Закрыть соединение|

|Настройка|Описание|
|---|---|
|`ws.text(true)`|Текстовый режим (vs бинарный)|
|`ws.binary(true)`|Бинарный режим|
|`ws.set_option(...)`|Настройки (заголовки, таймауты)|
|`ws.got_text()` / `ws.got_binary()`|Тип последнего полученного сообщения|

## 2.3 WebSocket
### src/test_beast.cpp

```cpp
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <format>

// ...

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
    }}  
  
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
        }    } catch (const boost::beast::system_error& se) {  
        if (se.code() != boost::beast::websocket::error::closed) {  
            std::cerr << std::format("Error: {}\n", se.code().message());  
        } else {  
            std::cout << std::format("Client closed connection\n");  
        }    } catch (const std::exception& e) {  
        std::cerr << std::format("Error: {}\n", e.what());  
    }}  
  
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
            };            std::cout << "WebSocket-server on port: ws://localhost:9001\n";  
  
            for (;;) {  
                boost::asio::ip::tcp::socket socket{io};  
                acceptor.accept(socket);  
                std::thread{do_ws_session, std::move(socket)}.detach();  
            }        } catch (const std::exception& e) {  
            std::cerr << std::format("Error: {}\n", e.what());  
        }    }};  
  
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));  
  
    t0.join();  
    t1.join();  
}
```

```
WebSocket-server on port: ws://localhost:9001
WS-connection established
Connecting to WS-server
Read: Hello, WS-server
Answer: Hello, WS-server
Connection closed
Client closed connection
```

## Часть 3. Вспомогательные средства

|Тип/функция|Описание|
|---|---|
|`beast::flat_buffer`|Динамический буфер для чтения сообщений|
|`beast::tcp_stream`|Сокет с встроенными таймаутами (`expires_after`)|
|`beast::make_printable(buf)`|Безопасный вывод содержимого буфера|
|`beast::error_code`|Псевдоним кода ошибки (= `boost::system::error_code`)|
|`beast::system_error`|Исключение с кодом ошибки|
|`http::error`|Коды ошибок HTTP (`end_of_stream` и др.)|
|`websocket::error`|Коды ошибок WebSocket (`closed` и др.)|

### Таймауты через `tcp_stream`

```cpp
beast::tcp_stream stream(io);
stream.expires_after(std::chrono::seconds(30)); // таймаут на следующую операцию
// если операция не уложится — завершится с ошибкой timeout
```

## Часть 4. HTTPS / WSS (кратко)

Для защищённых соединений Beast-стрим оборачивается в `ssl::stream` из Asio (требует OpenSSL):

```cpp
#include <boost/asio/ssl.hpp>
net::ssl::context ctx(net::ssl::context::tlsv12_client);
beast::ssl_stream<beast::tcp_stream> stream(io, ctx);
// ... handshake SSL, затем обычные HTTP/WS-операции поверх ...
```

vcpkg подтянет OpenSSL: добавь `"openssl"` в зависимости и слинкуй `OpenSSL::SSL OpenSSL::Crypto`.

## Сводка ключевых концепций

|Концепция|Суть|
|---|---|
|`http::request` / `http::response`|HTTP-сообщения, параметризованы Body-типом|
|Body-типы|`string_body`, `dynamic_body`, `file_body`, `empty_body`|
|`http::verb` / `http::status` / `http::field`|Методы, коды, заголовки|
|`http::read` / `http::write` (+async)|Чтение/запись сообщений целиком|
|`flat_buffer`|Буфер-накопитель для чтения|
|`prepare_payload()`|Согласование длины тела|
|`websocket::stream<T>`|WebSocket поверх любого стрима|
|`accept`/`handshake`|Серверное/клиентское рукопожатие WS|
|`read`/`write` (WS)|Обмен сообщениями целиком|
|`tcp_stream`|Сокет с таймаутами|
## Практические советы

- **Сначала синхронный клиент**, потом синхронный сервер, потом асинхронный/корутины — та же лестница, что в Asio.
- **`prepare_payload()` не забывай** перед отправкой: без него Content-Length может быть не выставлен, и клиент «зависнет» в ожидании тела.
- **Проверяй `http::error::end_of_stream`** — это нормальное завершение соединения, а не настоящая ошибка.
- **Beast — это слой протокола, а не фреймворк.** Роутинг, обработку ошибок, статику пишешь сам. Для быстрого REST API иногда удобнее взять Drogon/Crow; Beast выбирают за контроль, производительность и интеграцию с Asio.
- **Таймауты обязательны в продакшене** — используй `beast::tcp_stream` с `expires_after`, иначе медленный/зависший клиент будет держать ресурсы.
- **WebSocket-сообщения атомарны** на уровне Beast: один `read` даёт одно полное сообщение, фрейминг скрыт.

## Отличия от стандарта и альтернатив

- В стандартной библиотеке **HTTP/WebSocket нет** — Beast уникален в экосистеме Boost/Asio.
- Альтернативы: cpp-httplib (header-only, проще, но менее гибкая), libcurl (зрелый HTTP-клиент на C), Drogon/Crow (полноценные веб-фреймворки), Pistache, POCO.
- Сила Beast — глубокая интеграция с Asio (общая async-модель, корутины, strand'ы) и низкоуровневый контроль; цена — много ручной работы для высокоуровневых задач.
