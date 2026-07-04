---
tags:
  - programming-language
  - docker
---
# Producer / Consumer на Boost.Asio с корутинами C++20

### consumer/main.cpp
#### асинхронный echo-сервер
```cpp
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
            }            std::cout << std::format("Received: {}\n", received);  
  
            std::string response{std::format("ECHO: {}\n", received)};  
            co_await boost::asio::async_write(  
                socket,  
                boost::asio::buffer(response),  
                boost::asio::use_awaitable  
            );  
        }    } catch (const std::exception& e) {  
        // Обрыв соединения клиентом бросает исключение из async_read_some — это нормальный сценарий  
        std::cout << std::format("Client turned off: {}\n", e.what());  
    }}  
  
boost::asio::awaitable<void> listener() {  
    const int PORT{5000};  
  
    auto executor = co_await boost::asio::this_coro::executor;  
    boost::asio::ip::tcp::acceptor acceptor(  
        executor,  
        boost::asio::ip::tcp::endpoint(  
            boost::asio::ip::tcp::v4(),  
            PORT  
        )  
    );    std::cout << std::format("Consumer listen to port {}\n", PORT);  
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
    }}  
  
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
```

### consumer/CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.20)  
project(consumer LANGUAGES CXX)  
  
set(CMAKE_CXX_STANDARD 20)  
set(CMAKE_CXX_STANDARD_REQUIRED ON)  
  
find_package(Boost REQUIRED COMPONENTS system)  
find_package(Threads REQUIRED)  
  
add_executable(consumer main.cpp)  
target_link_libraries(consumer PRIVATE Boost::system Threads::Threads)
```

Обрати внимание: `-fcoroutines` отдельно указывать не нужно — начиная с GCC 11 корутины автоматически включаются флагом `-std=c++20` (в Alpine 3.20 стоит GCC 13+, так что всё ок).

### consumer/Dockerfile
```Dockerfile
# ---- этап сборки ----  
FROM alpine:3.20 AS builder  
  
RUN apk --no-cache build-base cmake boost-dev  
  
WORKDIR /src  
COPY CMakeLists.txt .  
COPY main.cpp .  
  
RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release && \  
    cmake --build build  
  
# ---- финальный этап ----  
FROM alpine:3.20  
  
# boost-dev тянет за собой headers + статику; в рантайме нужны только сами .so  
RUN apk add --no-cache libstdc++ boost-system  
  
WORKDIR /app  
COPY --from=builder /src/build/consumer .  
  
EXPOSE 5000  
ENTRYPOINT ["./consumer"]
```

### producer/main.cpp
#### асинхронный клиент
```cpp
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
    }    return 0;  
}
```

### producer/CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.20)  
project(producer LANGUAGES CXX)  
  
set(CMAKE_CXX_STANDARD 20)  
set(CMAKE_CXX_STANDARD_REQUIRED ON)  
  
find_package(Boost REQUIRED COMPONENTS system)  
find_package(Threads REQUIRED)  
  
add_executable(producer main.cpp)  
target_link_libraries(producer PRIVATE Boost::system Threads::Threads)
```

### producer/Dockerfile
```Dockerfile
FROM alpine:3.20 AS builder  
  
RUN apk add --no-cache build-base cmake boost-dev  
  
WORKDIR /src  
COPY CMakeLists.txt .  
COPY main.cpp .  
  
RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release && \  
    cmake --build build  
  
FROM alpine:3.20  
  
RUN apk add --no-cache libstdc++ boost-system  
  
WORKDIR /app  
COPY --from=builder /src/build/producer .  
  
ENTRYPOINT ["./producer"]
```

## Запуск — без изменений в командах

```powershell
docker network create mynet

docker build -t consumer:2.0 ./consumer
docker build -t producer:2.0 ./producer

docker run -d --name consumer --network mynet consumer:2.0
docker run --rm --network mynet producer:2.0 consumer "Привет через Asio!"
```

## Проверка конкурентности — то, чего не было в raw-версии

Старый сервер обрабатывал по одному клиенту за раз последовательно (блокирующий `accept`/`read`). Новый — обрабатывает клиентов конкурентно за счёт корутин на одном потоке `io_context`. Проверь это, запустив несколько producer'ов параллельно:

```powershell
docker run --rm --network mynet producer:2.0 consumer "Клиент 1" &
docker run --rm --network mynet producer:2.0 consumer "Клиент 2" &
docker run --rm --network mynet producer:2.0 consumer "Клиент 3" &
```

В логах `consumer` увидишь все три сообщения обработанными, хотя запущены они были практически одновременно — ни один клиент не блокирует остальных, ожидая своей очереди.
