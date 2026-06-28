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

// ...

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

// ...
  
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

## 8. Strands — синхронизация без мьютексов

Покажу проблему, которую решают strand'ы, и затем решение. Сценарий: несколько потоков крутят один `io_context`, и несколько асинхронных операций обращаются к **общему состоянию** одного объекта. Без strand'а обработчики могут выполняться параллельно и портить данные; strand сериализует их.

## Пример: счётчик, обновляемый несколькими таймерами

Несколько таймеров асинхронно инкрементируют общий счётчик. `io_context` запущен в пуле из 4 потоков, поэтому обработчики таймеров реально выполняются параллельно.

### include/test_asio.cpp
```cpp
#include <boost/asio.hpp>
#include <iostream>
#include <vector>
#include <thread>
#include <memory>

// ...

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
        // обработчики этого strand'а НЕ выполнятся одновременно        timer_.async_wait(boost::asio::bind_executor(  
            strand_,  
            [this, self](const boost::system::error_code& _ec) {  
                if (!_ec) on_tick();  
            }));    }  
    void on_tick() {  
        // Этот код защищён strand'ом: даже при 4 потоках в пуле  
        // два on_tick одного Counter НЕ выполнятся параллельно.        // Поэтому ++value_ безопасен БЕЗ мьютекса.  
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
    }}
```

```
Final value: 13
Final value: 13
Final value: 13
```


## Где здесь strand и что он гарантирует

**Создание:**

```cpp
strand_(boost::asio::make_strand(io))
```

`make_strand` создаёт strand, связанный с executor'ом `io_context`.

**Применение:**

```cpp
boost::asio::bind_executor(strand_, handler)
```

Обёрнутый так обработчик попадает в очередь strand'а. Asio гарантирует: **два обработчика одного strand'а никогда не выполняются одновременно**, даже если `io.run()` крутят несколько потоков.

**Результат:** внутри `on_tick()` обращение к `value_` безопасно без мьютекса — strand уже сериализовал доступ. Каждый `Counter` имеет свой strand, поэтому разные счётчики могут обновляться параллельно (они не делят состояние), а внутри одного счётчика — строго по очереди.

## Ключевые функции для работы со strand

| Функция                                     | Назначение                                           |
| ------------------------------------------- | ---------------------------------------------------- |
| `make_strand(io)` / `make_strand(executor)` | Создать strand                                       |
| `bind_executor(strand, handler)`            | Привязать обработчик к strand'у (для `async_*`)      |
| `post(strand, handler)`                     | Поставить обработчик в очередь strand'а немедленно   |
| `dispatch(strand, handler)`                 | То же, но может выполнить сразу, если уже в strand'е |

## Когда нужен strand

|Ситуация|Нужен ли strand|
|---|---|
|`io.run()` в **одном** потоке|Нет — обработчики и так не пересекаются|
|Пул потоков + объект без общего состояния|Нет|
|Пул потоков + общее изменяемое состояние / один сокет|**Да**|
|Несколько независимых объектов|По strand'у на объект|
## Главное про strand

- Strand **сериализует** обработчики: они не выполняются одновременно, но порядок постановки сохраняется.
- Это альтернатива мьютексам внутри async-кода: **без блокировок, без риска deadlock**.
- Один strand — на один логический объект с общим состоянием (счётчик, соединение).
- Применяется через `bind_executor` (для async-операций) или `post`/`dispatch` (для отложенного вызова).
- Имеет смысл **только** при многопоточном `io_context`; при однопоточном `run()` он избыточен.

## 9. Корутины (современный стиль, C++20)

Полноценный эхо-сервер на корутинах плюс клиент, тоже на корутинах. Корутины убирают «лапшу» из вложенных колбэков: асинхронный код читается линейно, как синхронный, но не блокирует поток.

```cpp
#include <boost/asio.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

// ...
// --- SERVER ---  
static boost::asio::awaitable<void> handle_session(boost::asio::ip::tcp::socket _socket) {  
    try {  
        char data[1024];  
        for (;;) {  
            // co_await приостанавливает корутину, не блокируя поток.  
            // Когда данные придут, выполнение продолжится с этой строки.            std::size_t n{co_await _socket.async_read_some(  
                boost::asio::buffer(data),  
                boost::asio::use_awaitable)};  
  
            std::cout << std::format("[SERVER] Took: {}\n", std::string(data, n));  
  
            // отправляем эхо обратно — снова co_await  
            co_await boost::asio::async_write(  
                _socket,  
                boost::asio::buffer(data, n),  
                boost::asio::use_awaitable);  
        }    } catch (const std::exception& e) {  
        // при закрытии соединения async_read_some бросит исключение (EOF)  
        std::cout << std::format("[SERVER] Session closed: {}\n", e.what());  
    }}  
  
// --- SERVER ---  
  
static boost::asio::awaitable<void> listener(boost::asio::io_context& _io, unsigned short _port) {  
    boost::asio::ip::tcp::acceptor acceptor(  
        _io,  
        boost::asio::ip::tcp::endpoint(  
            boost::asio::ip::tcp::v4(),  
            _port  
        )  
    );    std::cout << std::format("[SERVER] listen to port {}\n", _port);  
  
    for (;;) {  
        // ждём входящее соединение асинхронно  
        boost::asio::ip::tcp::socket socket = co_await acceptor.async_accept(  
            boost::asio::use_awaitable  
        );  
        std::cout << std::format("[SERVER] New connection\n");  
  
        // запускаем обработку сессии как отдельную корутину;  
        // detached — нам не нужен её результат, она живёт сама по себе        boost::asio::co_spawn(_io, handle_session(std::move(socket)), boost::asio::detached);  
    }}  
  
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
    }}  
  
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
    }}

```

```
[SERVER] listen to port 12345
[SERVER] New connection
[CLIENT] Connected
[SERVER] Took: Message #1

[CLIENT] Send: Message #1

[CLIENT] Echo: Message #1

[SERVER] Took: Message #2

[CLIENT] Send: Message #2

[CLIENT] Echo: Message #2

[SERVER] Took: Message #3

[CLIENT] Send: Message #3

[CLIENT] Echo: Message #3

[SERVER] Took: Message #4

[CLIENT] Send: Message #4

[CLIENT] Echo: Message #4

[SERVER] Took: Message #5

[CLIENT] Send: Message #5

[CLIENT] Echo: Message #5

[CLIENT] Shutting down...
[SERVER] Session closed: End of file [asio.misc:2 at C:/projects/knowledge_base/programming languages/cpp/boost/code/.build/vcpkg_installed/x64-windows/include\boost/asio/detail/win_iocp_socket_recv_op.hpp:90:33 in function 'static void __cdecl boost::asio::detail::win_iocp_socket_recv_op<boost::asio::mutable_buffer, boost::asio::detail::awaitable_handler<boost::asio::any_io_executor, boost::system::error_code, unsigned long long>, boost::asio::any_io_executor>::do_complete(void *, operation *, const boost::system::error_code &, std::size_t) [MutableBufferSequence = boost::asio::mutable_buffer, Handler = boost::asio::detail::awaitable_handler<boost::asio::any_io_executor, boost::system::error_code, unsigned long long>, IoExecutor = boost::asio::any_io_executor]']
[CLIENT] Done!
```

## Ключевые элементы корутинной модели

### `awaitable<T>`

Тип возвращаемого значения корутины Asio. `awaitable<void>` — корутина без результата, `awaitable<std::size_t>` — возвращающая число и т.д. Функция, возвращающая `awaitable<...>` и содержащая `co_await`/`co_return`, и есть корутина.

### `co_await` + `use_awaitable`

```cpp
std::size_t n = co_await socket.async_read_some(buffer, use_awaitable);
```

`use_awaitable` — это **completion token**, говорящий Asio: «оформи эту async-операцию как awaitable». `co_await` приостанавливает корутину до завершения операции, **освобождая поток** для другой работы. Когда операция завершится, выполнение продолжится со следующей строки, а результат вернётся как обычное значение.

### `co_spawn`

```cpp
co_spawn(io, my_coroutine(), detached);
```

Запускает корутину на исполнение в `io_context`. Третий аргумент — completion token для результата самой корутины:

|Токен|Поведение|
|---|---|
|`detached`|Результат игнорируется, корутина живёт сама|
|лямбда/функция|Колбэк с результатом/исключением корутины|
|`use_awaitable`|Дождаться из другой корутины (`co_await co_spawn(...)`)|

### Обработка ошибок через исключения

В корутинном стиле ошибки приходят как **исключения**, а не через `error_code`. Поэтому тело оборачивают в `try/catch`. Например, при закрытии соединения `async_read_some` бросит исключение — это нормальный сигнал завершения сессии.

> Альтернатива: можно получать `error_code` вместо исключений, используя токен `as_tuple(use_awaitable)` — тогда операция вернёт `std::tuple<error_code, ...>` и не бросит.

## Сравнение: тот же сервер на колбэках vs корутинах

**Колбэки** (как в раннем примере) — логика размазана по методам `read()` и `write()`, каждый вызывает следующий через вложенные лямбды, плюс нужен `shared_from_this()` для продления жизни объекта.

**Корутины** — вся логика сессии в одной функции `handle_session`, читается сверху вниз: прочитал → записал → повторил. Никаких ручных колбэков и `shared_from_this`: локальные переменные (`data`, `socket`) живут на «кадре» корутины автоматически.

```cpp
// Колбэки: «что дальше» прячется в следующей лямбде
void read() {
    socket_.async_read_some(buf, [self](auto ec, auto n){
        if (!ec) write(n);   // переход к следующему шагу
    });
}

// Корутина: «что дальше» — просто следующая строка
for (;;) {
    auto n = co_await socket.async_read_some(buf, use_awaitable);
    co_await async_write(socket, buffer(data, n), use_awaitable);
}
```

## CMake и требования

```cmake
cmake_minimum_required(VERSION 3.21)
project(asio_coro LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)         # корутины требуют C++20
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)

add_executable(app main.cpp)
target_link_libraries(app PRIVATE Boost::system Threads::Threads)
```

Требования:

- **C++20** обязателен (`-std=c++20` / `/std:c++20`).
- Компиляторы: GCC 10+, Clang 14+, MSVC 2019 16.8+.
- Boost достаточно свежий (1.70+, лучше новее — поддержка корутин в Asio активно дорабатывалась).

## Почему это однопоточно и всё работает

Обрати внимание: `io.run()` вызван из **одного** потока, но сервер и клиент работают «одновременно». Это и есть суть асинхронности: пока одна корутина ждёт на `co_await` (данные из сети, срабатывание таймера), поток не простаивает — он исполняет другую готовую корутину. Конкурентность без параллелизма. При необходимости можно добавить пул потоков (`io.run()` из нескольких), но тогда для общего состояния снова понадобятся strand'ы.

## Когда выбирать корутины

|Стиль|Когда|
|---|---|
|Корутины (`co_await`)|Новый код на C++20+; сложная последовательная логика; читаемость в приоритете|
|Колбэки|Старые компиляторы (C++03/11/14); простые одношаговые операции; интеграция с готовым колбэк-кодом|
|`use_future`|Нужен `std::future` для интеграции с другим асинхронным кодом|

Для нового кода на C++20 корутины — рекомендуемый подход: они дают тот же асинхронный движок, что и колбэки, но без визуального усложнения и без ручного управления временем жизни объектов.

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

## 12. Прочие возможности

|Возможность|Тип|
|---|---|
|Последовательные порты|`serial_port`|
|Сигналы ОС|`signal_set` (например, перехват SIGINT)|
|POSIX-дескрипторы|`posix::stream_descriptor`|
|Пайпы|`readable_pipe` / `writable_pipe`|
|SSL/TLS|`ssl::stream` (заголовок `<boost/asio/ssl.hpp>`, требует OpenSSL)|

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
## Практические советы

- **Начинай с таймеров**, затем синхронный клиент, потом асинхронный, и только потом сервер. Так модель усваивается постепенно.
- **`shared_from_this` — обязательный паттерн** для async-серверов: забудешь — получишь обращение к уничтоженному объекту.
- **Для нового кода рассматривай корутины** (`co_await` + `use_awaitable`) — они радикально упрощают чтение по сравнению с колбэками. Нужен C++20.
- **`error_code` всегда проверяй** в обработчиках — асинхронная операция могла завершиться ошибкой (разрыв соединения, отмена).
- **Один `io_context` на несколько потоков** + `strand` — типичная схема масштабирования; не разделяй один сокет между потоками без strand.
- **Asio — основа Beast.** Уверенно разберись здесь, прежде чем переходить к HTTP/WebSocket.

## Отличия от стандарта и контекст

- Asio долго был кандидатом на стандартизацию (Networking TS), но в C++ так и **не вошёл** на момент C++23 — поэтому Boost.Asio (или его standalone-версия без Boost) остаётся стандартом де-факто для сетей в C++.
- Существует **standalone Asio** (без зависимости от Boost) — тот же автор (Christopher Kohlhoff), почти идентичный API в namespace `asio::` вместо `boost::asio::`.
- В стандартной библиотеке сетевых средств нет; ближайшее — сторонние библиотеки (libuv для C, POCO, Qt Network).
