---
tags:
  - programming-language
  - docker
---

# Полный `docker-compose.yml` для нашего проекта

### consumer.main.cpp
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

### producer/main.cpp
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

### Dockerfile
```Dockerfile
# ---- общая база: тяжёлые build-инструменты ставим один раз ----  
FROM alpine:3.20 AS base  
RUN apk add --no-cache build-base cmake boost-dev  
  
# ---- параллельная стадия: сборка consumer ----  
FROM base AS builder-consumer  
WORKDIR /src  
COPY consumer/CMakeLists.txt .  
COPY consumer/main.cpp .  
RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release && \  
    cmake --build build  
  
# ---- параллельная стадия: сборка producer ----  
FROM base AS builder-producer  
WORKDIR /src  
COPY producer/CMakeLists.txt .  
COPY producer/main.cpp .  
RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release && \  
    cmake --build build  
  
# ---- финальный образ: только consumer ----  
FROM alpine:3.20 AS consumer  
RUN apk add --no-cache libstdc++ boost-system  
WORKDIR /app  
COPY --from=builder-consumer /src/build/consumer .  
EXPOSE 5000  
ENTRYPOINT ["./consumer"]  
  
# ---- финальный образ: только producer ----  
FROM alpine:3.20 AS producer  
RUN apk add --no-cache libstdc++ boost-system  
WORKDIR /app  
COPY --from=builder-producer /src/build/producer .  
ENTRYPOINT ["./producer"]  
  
# ---- демонстрационная стадия: оба бинарника сразу ----  
# Это ПОСЛЕДНЯЯ стадия в файле — именно её Docker соберёт по умолчанию,  
# без явного --target. Раз она зависит и от builder-consumer, и от  
# builder-producer — обе эти стадии будут собираться в рамках ОДНОГО  
# вызова docker build, и вот тут параллелизм BuildKit станет заметен.  
FROM alpine:3.20 AS both  
RUN apk add --no-cache libstdc++ boost-system  
WORKDIR /app  
COPY --from=builder-consumer /src/build/consumer .  
COPY --from=builder-producer /src/build/producer .
```

### docker-compose.yml
```yml
services:  
  consumer:  
    build:  
      context: .  
      target: consumer  
    ports:  
      - "5000:5000"  
    networks:  
      - mynet  
    healthcheck:  
      test: ["CMD", "nc", "-z", "127.0.0.1", "5000"]  
      interval: 2s  
      timeout: 2s  
      retries: 5  
    restart: unless-stopped  
  
  producer:  
    build:  
      context: .  
      target: producer  
    depends_on:  
      consumer:  
        condition: service_healthy  
    networks:  
      - mynet  
    # producer одноразовый — запускается, шлёт запрос, завершается.  
    # command переопределяет ENTRYPOINT/CMD, если нужно передать другие аргументы    command: ["consumer", "Привет из Compose!"]  
  
networks:  
  mynet:
```

## Проверка на практике

Положи `docker-compose.yml` рядом с папками `consumer/` и `producer/` (на уровне `006 combined build` или как у тебя структурировано), и выполни:

```powershell
docker compose up --build
```

Ожидаемо увидишь в логах вперемешку (с префиксами `consumer-1 |` и `producer-1 |`):

```
consumer-1  | Consumer слушает порт 5000...
consumer-1  | Получено: Привет из Compose!
producer-1  | Подключились к consumer:5000, отправляем: Привет из Compose!
producer-1  | Ответ: ECHO: Привет из Compose!
producer-1 exited with code 0
```

Обрати внимание: `consumer` продолжает работать (у него `restart: unless-stopped`), а `producer` завершается после одного запроса — это и есть декларативное описание того самого сценария, который раньше требовал пяти отдельных команд.

Останови и убери всё одной командой:

```powershell
docker compose down
```
