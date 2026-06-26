---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/boost/_|<=]]

# Boost.Asio

Boost.Asio — библиотека для асинхронного ввода/вывода: сети (TCP/UDP), таймеры, последовательные порты, сигналы. Сердцевина сетевого программирования на Boost и фундамент для Beast. В основном header-only, но требует системных библиотек (потоки; на Windows — winsock).

```cpp
#include <boost/asio.hpp>
namespace net = boost::asio; // частое сокращение
```

CMake:

```cmake
find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)
target_link_libraries(app PRIVATE Boost::system Threads::Threads)
```

## Центральная идея: модель Proactor

Asio построен на **асинхронной модели**: ты инициируешь операцию (`async_*`) и передаёшь **completion handler** (колбэк), который вызовется по завершении. Всё крутится вокруг `io_context`, который доставляет результаты завершённых операций обработчикам.

Поток управления:

1. Создаёшь `io_context`.
2. Регистрируешь асинхронные операции с обработчиками.
3. Вызываешь `io_context.run()` — он блокируется и исполняет обработчики по мере готовности операций.
4. Когда незавершённых операций не остаётся, `run()` возвращается.

## 1. `io_context` — ядро

Управляет очередью операций и их диспетчеризацией. Старое название — `io_service`.

|Метод|Описание|
|---|---|
|`run()`|Блокируется, исполняет обработчики, пока есть работа|
|`run_one()`|Исполнить один обработчик|
|`poll()`|Исполнить готовые обработчики **без** блокировки|
|`poll_one()`|Один готовый обработчик без блокировки|
|`stop()`|Принудительно остановить `run()`|
|`stopped()`|Проверка остановки|
|`restart()`|Сбросить после `stop()` для повторного `run()`|

```cpp
boost::asio::io_context io;
// ... регистрация операций ...
io.run(); // запуск цикла обработки
```

## 2. Поддержание работы: `executor_work_guard`

По умолчанию `run()` завершается, когда нет операций. Чтобы `io_context` не останавливался (например, в пуле потоков, ждущем будущих задач):

```cpp
auto work = boost::asio::make_work_guard(io);
// теперь io.run() не выйдет, пока work жив
work.reset(); // отпустить — позволить run() завершиться
```

## 3. Таймеры

Самый простой способ понять асинхронную модель.

|Тип|Описание|
|---|---|
|`steady_timer`|На основе монотонных часов (рекомендуется для интервалов)|
|`system_timer`|На системных часах (привязка к календарному времени)|
|`high_resolution_timer`|Высокого разрешения|
|`deadline_timer`|Старый (из Boost.Date_Time)|

| Метод                     | Описание                               |
| ------------------------- | -------------------------------------- |
| `expires_after(duration)` | Установить срабатывание через интервал |
| `expires_at(time_point)`  | Установить на конкретный момент        |
| `wait()`                  | Синхронное ожидание (блокирует)        |
| `async_wait(handler)`     | Асинхронное ожидание                   |
| `cancel()`                | Отменить ожидающие операции            |

### include/test_asio.cpp
```cpp
#include <boost/asio.hpp>  
#include <chrono>  
#include <iostream>

// ...

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
```

```
Wait...
The timer went off after 2 seconds!
```

## 4. Сеть: TCP

### Ключевые типы (namespace `boost::asio::ip`)

|Тип|Описание|
|---|---|
|`tcp::socket`|TCP-сокет|
|`tcp::acceptor`|Принимает входящие соединения (сервер)|
|`tcp::resolver`|Преобразует имя хоста + порт в endpoint'ы (DNS)|
|`tcp::endpoint`|Адрес + порт|
|`tcp::v4()` / `tcp::v6()`|Версия протокола|
|`ip::address`|IP-адрес|

### Операции сокета

|Sync|Async|Описание|
|---|---|---|
|`connect()`|`async_connect()`|Подключиться|
|`read_some()`|`async_read_some()`|Прочитать (сколько есть)|
|`write_some()`|`async_write_some()`|Записать (сколько влезло)|
|`accept()` (у acceptor)|`async_accept()`|Принять соединение|
|`close()`|—|Закрыть сокет|
|`shutdown()`|—|Завершить приём/передачу|

### Свободные функции для надёжного чтения/записи

`read_some`/`write_some` могут передать **часть** данных. Для полной передачи используют:

|Функция|Описание|
|---|---|
|`boost::asio::read(...)`|Прочитать ровно N байт|
|`boost::asio::write(...)`|Записать все данные|
|`boost::asio::read_until(...)`|Читать до разделителя (например, `\n`)|
|`boost::asio::async_read(...)`|Асинхронно прочитать ровно N|
|`boost::asio::async_write(...)`|Асинхронно записать всё|
|`boost::asio::async_read_until(...)`|Асинхронно до разделителя|

## 5. Буферы

| Тип/функция                        | Описание                                     |
| ---------------------------------- | -------------------------------------------- |
| `boost::asio::buffer(data, size)`  | Обёртка над сырым буфером                    |
| `boost::asio::buffer(std::string)` | Из строки                                    |
| `boost::asio::buffer(std::vector)` | Из вектора                                   |
| `streambuf`                        | Динамический буфер (для `read_until` и т.п.) |
| `dynamic_buffer(...)`              | Адаптер динамического буфера над контейнером |

### include/test_asio.cpp
```cpp
#include <boost/asio.hpp>  
#include <iostream>

// ...

void test_buffer() {  
    std::string message{"Hello"};  
    auto buf = boost::asio::buffer(message);  
  
    std::cout << "Buffer size: " << buf.size() << "\n";  
    std::cout << "Data: " << std::string(static_cast<const char*>(buf.data()), buf.size()) << "\n";  
}
```

```
Buffer size: 5
Data: Hello
```

## 6. Синхронный TCP-клиент (простейший старт)

### include/test_asio.cpp
```cpp
#include <boost/asio.hpp>
#include <iostream>

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
```

```
EC: End of file
Response:
HTTP/1.1 200 OK
Date: Fri, 26 Jun 2026 14:06:59 GMT
Content-Type: text/html
Transfer-Encoding: chunked
Connection: close
Server: cloudflare
Last-Modified: Sat, 20 Jun 2026 20:45:06 GMT
Allow: GET, HEAD
Accept-Ranges: bytes
Age: 3995
cf-cache-status: HIT
CF-RAY: a11cc6952b508db3-HEL

22f
<!doctype html><html lang="en"><head><title>Example Domain</title><link rel="icon" href="data:,"><meta name="viewport" content="width=device-width, initial-scale=1"><style>body{background:#eee;width:60vw;margin:15vh auto;font-family:system-ui,sans-serif}h1{font-size:1.5em}div{opacity:0.8}a:link,a:visited{color:#348}</style></head><body><div><h1>Example Domain</h1><p>This domain is for use in documentation examples without needing permission. Avoid use in operations.</p><p><a href="https://iana.org/domains/example">Learn more</a></p></div></body></html>

0
```

## 7. Асинхронный TCP-сервер (эхо)

Демонстрирует `async_accept`, `enable_shared_from_this` и цепочки обработчиков.

> Паттерн `shared_from_this()` критичен: он не даёт объекту `Session` уничтожиться, пока выполняется асинхронная операция, ссылающаяся на него.

### include/test_asio.cpp
```cpp
#include <boost/asio.hpp>
#include <memory>
#include <iostream>
#include <thread>
#include <chrono>
#include <array>

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
                }            }        );    }  
    void write(std::size_t _length) {  
        auto self = shared_from_this();  
        boost::asio::async_write(  
            socket_,  
            boost::asio::buffer(data_, _length),  
            [this, self](boost::system::error_code _ec, std::size_t) {  
                if (!_ec) read();  
            }        );    }  
    boost::asio::ip::tcp::socket socket_;  
    std::array<char, 1024> data_;  
};  
  
class Server {  
public:  
    Server(boost::asio::io_context& _io, unsigned short _port):  
        acceptor_{_io, boost::asio::ip::tcp::endpoint{boost::asio::ip::tcp::v4(), _port}} {  
        accept();  
    }private:  
    void accept() {  
        acceptor_.async_accept(  
            [this](boost::system::error_code _ec, boost::asio::ip::tcp::socket _socket) {  
                if (!_ec) {  
                    std::cout << "[SERVER] New connection\n";  
                    std::make_shared<Session>(std::move(_socket))->start();  
                }                accept();  
            }        );    }    boost::asio::ip::tcp::acceptor acceptor_;  
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
    }}  
  
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
```

```
Server is listening to port 12345                                                                                                                                                           
[CLIENT] Connected to server                                                                                                                                                                
[SERVER] New connection                                                                                                                                                                     
[CLIENT] Send: Hello 0                                                                                                                                                                      
[SERVER] Received: Hello 0                                                                                                                                                                  
[CLIENT] Echo: Hello 0                                                                                                                                                                      
[SERVER] Received: Hello 1                                                                                                                                                                  
[CLIENT] Send: Hello 1                                                                                                                                                                      
[CLIENT] Echo: Hello 1                                                                                                                                                                      
[CLIENT] Send: Hello 2                                                                                                                                                                      

[SERVER] Received: Hello 2

[CLIENT] Echo: Hello 2

[CLIENT] Send: Hello 3

[SERVER] Received: Hello 3

[CLIENT] Echo: Hello 3

[CLIENT] Send: Hello 4

[SERVER] Received: Hello 4

[CLIENT] Echo: Hello 4

[SERVER] Received: Hello 5

[CLIENT] Send: Hello 5

[CLIENT] Echo: Hello 5

[SERVER] Connection closed: End of file
[CLIENT] Shutting down...
DONE!
```


## Важные нюансы потокобезопасности

- **`std::cout` из двух потоков** может «склеивать» строки. В учебном примере это терпимо; в реальном коде вывод защищают мьютексом или используют отдельный логгер.
- **Один сокет нельзя дёргать из разных потоков** одновременно. Здесь проблемы нет: серверный сокет живёт в потоке `io.run()`, клиентский — в потоке `client_thread`, они независимы.
- **Если бы клиент тоже был асинхронным** и делил `io_context` с сервером при нескольких потоках `run()` — понадобился бы `strand` для сериализации обработчиков одного сокета.

## Вариант с несколькими потоками на сервере

Если хочешь, чтобы сервер обрабатывал соединения в пуле потоков, запускают `io.run()` из нескольких потоков:

```cpp
std::vector<std::thread> pool;
for (int i = 0; i < 4; ++i)
    pool.emplace_back([&io]{ io.run(); });
for (auto& th : pool) th.join();
```

Но тогда для каждого `Session`, к которому могут обращаться разные потоки, нужен `strand`, оборачивающий его обработчики, — иначе чтение и запись на одном сокете могут пойти параллельно.



---
---
---
---

## 8. Strands — синхронизация без мьютексов

Когда несколько потоков вызывают `io.run()`, обработчики могут выполняться параллельно. `strand` гарантирует, что обработчики, отправленные через него, **не пересекаются** во времени:

```cpp

//< !!! более развернутый пример

boost::asio::strand<boost::asio::io_context::executor_type> strand(io.get_executor());

// Обработчики, обёрнутые в strand, не выполнятся одновременно
timer.async_wait(boost::asio::bind_executor(strand, handler));
```

Это даёт потокобезопасность без явных мьютексов в обработчиках одного логического объекта.

---

## 9. Корутины (современный стиль, C++20)

Asio поддерживает `co_await` — асинхронный код выглядит как синхронный:

```cpp

//< !!!

#include <boost/asio.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>

using boost::asio::awaitable;
using boost::asio::use_awaitable;
using boost::asio::ip::tcp;

awaitable<void> echo(tcp::socket socket) {
    char data[1024];
    for (;;) {
        std::size_t n = co_await socket.async_read_some(
            boost::asio::buffer(data), use_awaitable);
        co_await boost::asio::async_write(
            socket, boost::asio::buffer(data, n), use_awaitable);
    }
}

// запуск: co_spawn(io, echo(std::move(socket)), boost::asio::detached);
```

> Корутины — рекомендуемый современный подход: убирают «лапшу» из вложенных колбэков.

---

## 10. Completion tokens — гибкость стиля

Asio-операции принимают **completion token**, определяющий, как вернуть результат:

| Токен                   | Стиль                              |
| ----------------------- | ---------------------------------- |
| Лямбда/функция          | Классический колбэк                |
| `use_awaitable`         | Корутины (`co_await`)              |
| `use_future`            | Вернуть `std::future`              |
| `boost::asio::detached` | Игнорировать результат             |
| `yield_context`         | Стэкфул-корутины (Boost.Coroutine) |

Один и тот же `async_read` можно использовать в любом стиле, меняя только токен.

```cpp
//< !!!
```

---

## 11. UDP (кратко)

|Тип|Описание|
|---|---|
|`udp::socket`|UDP-сокет|
|`udp::endpoint`|Адрес+порт|
|`udp::resolver`|Резолвер|

|Операция|Описание|
|---|---|
|`send_to()` / `async_send_to()`|Отправить датаграмму|
|`receive_from()` / `async_receive_from()`|Принять датаграмму|

UDP не требует установки соединения — работаешь датаграммами напрямую.

---

## 12. Прочие возможности

|Возможность|Тип|
|---|---|
|Последовательные порты|`serial_port`|
|Сигналы ОС|`signal_set` (например, перехват SIGINT)|
|POSIX-дескрипторы|`posix::stream_descriptor`|
|Пайпы|`readable_pipe` / `writable_pipe`|
|SSL/TLS|`ssl::stream` (заголовок `<boost/asio/ssl.hpp>`, требует OpenSSL)|

---

## Сводка ключевых концепций

|Концепция|Суть|
|---|---|
|`io_context`|Ядро, диспетчер обработчиков; `run()` запускает цикл|
|Completion handler|Колбэк, вызываемый по завершении `async_*` операции|
|`work_guard`|Удерживает `io_context` от остановки|
|Таймеры|`steady_timer` — простейший вход в async-модель|
|`tcp::socket`/`acceptor`/`resolver`|Сетевые примитивы|
|`buffer()`|Обёртка данных для I/O|
|`async_read`/`async_write`|Надёжная передача всех данных|
|`shared_from_this`|Продление жизни объекта в async-операциях|
|`strand`|Сериализация обработчиков без мьютексов|
|Корутины (`co_await`)|Современный плоский async-код|
|Completion tokens|Выбор стиля: колбэк / future / корутина|

---

## Практические советы

- **Начинай с таймеров**, затем синхронный клиент, потом асинхронный, и только потом сервер. Так модель усваивается постепенно.
- **`shared_from_this` — обязательный паттерн** для async-серверов: забудешь — получишь обращение к уничтоженному объекту.
- **Для нового кода рассматривай корутины** (`co_await` + `use_awaitable`) — они радикально упрощают чтение по сравнению с колбэками. Нужен C++20.
- **`error_code` всегда проверяй** в обработчиках — асинхронная операция могла завершиться ошибкой (разрыв соединения, отмена).
- **Один `io_context` на несколько потоков** + `strand` — типичная схема масштабирования; не разделяй один сокет между потоками без strand.
- **Asio — основа Beast.** Уверенно разберись здесь, прежде чем переходить к HTTP/WebSocket.

---

## Отличия от стандарта и контекст

- Asio долго был кандидатом на стандартизацию (Networking TS), но в C++ так и **не вошёл** на момент C++23 — поэтому Boost.Asio (или его standalone-версия без Boost) остаётся стандартом де-факто для сетей в C++.
- Существует **standalone Asio** (без зависимости от Boost) — тот же автор (Christopher Kohlhoff), почти идентичный API в namespace `asio::` вместо `boost::asio::`.
- В стандартной библиотеке сетевых средств нет; ближайшее — сторонние библиотеки (libuv для C, POCO, Qt Network).



---
---
---

## Этап 6. Сеть и асинхронность

### 6.1 Boost.Asio

Асинхронный ввод/вывод, таймеры, сокеты. Сердцевина сетевого программирования на Boost. В основном header-only, но требует системных библиотек (на Windows — ws2_32, на Linux — pthread; vcpkg/CMake подтянут зависимости автоматически).

Начни с синхронного примера, затем переходи к асинхронному.

```cpp
#include <boost/asio.hpp>
#include <iostream>

int main() {
    boost::asio::io_context io;
    boost::asio::steady_timer timer(io, std::chrono::seconds(2));

    timer.async_wait([](const boost::system::error_code& ec) {
        if (!ec) std::cout << "Таймер сработал!\n";
    });

    std::cout << "Ожидание...\n";
    io.run(); // блокируется, пока есть незавершённые операции
}
```

CMake:

```cmake
find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)
target_link_libraries(app PRIVATE Boost::system Threads::Threads)
```

**Ключевое:** `io_context` и его роль, модель completion handlers, `async_*` операции, strands для синхронизации, корутины (`co_await` с `boost::asio::awaitable`). Это фундамент для Beast — изучай тщательно.

---
---
---

### 6.2 Boost.Beast

HTTP и WebSocket поверх Asio. Изучается **только после** уверенного владения Asio. Header-only, зависит от Asio.

```cpp
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <iostream>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

int main() {
    net::io_context io;
    tcp::resolver resolver(io);
    beast::tcp_stream stream(io);

    auto results = resolver.resolve("example.com", "80");
    stream.connect(results);

    http::request<http::string_body> req{http::verb::get, "/", 11};
    req.set(http::field::host, "example.com");
    req.set(http::field::user_agent, "Beast");
    http::write(stream, req);

    beast::flat_buffer buffer;
    http::response<http::dynamic_body> res;
    http::read(stream, buffer, res);
    std::cout << res.base() << "\n"; // заголовки ответа

    beast::error_code ec;
    stream.socket().shutdown(tcp::socket::shutdown_both, ec);
}
```

CMake:

```cmake
find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)
target_link_libraries(app PRIVATE Boost::system Threads::Threads)
```

**Ключевое:** структуры `request`/`response`, синхронный HTTP-клиент → асинхронный → простой HTTP-сервер → WebSocket. Самый практичный модуль для веб-задач.

---
---
---
---

## Этап 7. Тестирование

### 7.1 Boost.Test

Фреймворк юнит-тестирования. **Требует линковки** (для unit_test_framework). Изучи раньше остальных по желанию — полезно писать тесты ко всем учебным примерам.

```cpp
#define BOOST_TEST_MODULE MyTests
#include <boost/test/included/unit_test.hpp>

int add(int a, int b) { return a + b; }

BOOST_AUTO_TEST_CASE(addition_works) {
    BOOST_CHECK_EQUAL(add(2, 3), 5);
    BOOST_TEST(add(-1, 1) == 0);
}

BOOST_AUTO_TEST_CASE(edge_cases) {
    BOOST_CHECK(add(0, 0) == 0);
}
```

CMake (вариант со скомпилированной библиотекой):

```cmake
find_package(Boost REQUIRED COMPONENTS unit_test_framework)
enable_testing()
add_executable(tests test_main.cpp)
target_link_libraries(tests PRIVATE Boost::unit_test_framework)
add_test(NAME tests COMMAND tests)
```

Запуск: `ctest --test-dir build --output-on-failure`.

**Ключевое:** `BOOST_CHECK` против `BOOST_REQUIRE`, `BOOST_TEST`, test suites, fixtures, интеграция с CTest. Совет: используй `<boost/test/included/...>` для header-only режима в маленьких проектах и линкуемую версию — в больших.

---
---
---
---

## Этап 8. Интеграция с Python

### 8.1 Boost.Python

Связывание C++ и Python. **Требует линковки** и установленного Python. Самый зависимый от окружения модуль — оставь напоследок.

```cpp
// hello_ext.cpp
#include <boost/python.hpp>

char const* greet() { return "Привет из C++!"; }
int square(int x) { return x * x; }

BOOST_PYTHON_MODULE(hello_ext) {
    using namespace boost::python;
    def("greet", greet);
    def("square", square);
}
```

CMake (сборка как разделяемая библиотека-модуль):

```cmake
find_package(Boost REQUIRED COMPONENTS python)
find_package(Python3 REQUIRED COMPONENTS Development)

add_library(hello_ext MODULE hello_ext.cpp)
target_link_libraries(hello_ext PRIVATE Boost::python Python3::Python)
set_target_properties(hello_ext PROPERTIES PREFIX "" SUFFIX ".pyd") # Windows
# На Linux: SUFFIX ".so"
```

Использование из Python:

```python
import hello_ext
print(hello_ext.greet())
print(hello_ext.square(7))
```

**Ключевое:** экспорт функций, классов (`class_<>`), конвертеры типов, управление GIL. Альтернатива — pybind11 (легче, header-only), стоит знать о ней.

---

