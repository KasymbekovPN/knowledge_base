---
tags:
  - programming-language
  - docker
---
# Практика: Producer / Consumer на C++ через user-defined bridge

Структура проекта:

```
.../
├── consumer/
│   ├── CMakeLists.txt
│   ├── main.cpp
│   └── Dockerfile
└── producer/
    ├── CMakeLists.txt
    ├── main.cpp
    └── Dockerfile
```

Используем обычные POSIX-сокеты (`sys/socket.h`) — без Boost.Asio, чтобы не тащить лишние зависимости в Alpine-образ. Раз оба этапа сборки будут на Alpine (musl), проблемы musl/glibc, о которой я предупреждал раньше, тут не будет — компилируем и запускаем в одном и том же окружении.

## consumer/main.cpp
#### сервер, отвечает `ECHO: ...`

```cpp
#include <arpa/inet.h>  
#include <cstring>  
#include <iostream>  
#include <format>  
#include <netinet/in.h>  
#include <string>  
#include <sys/socket.h>  
#include <unistd.h>  
  
int main() {  
    const int PORT{5000};  
  
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);  
    if (server_fd < 0) {  
        std::cerr << "Error creating socket\n";  
        return 1;  
    }  
    // Разрешаем повторное использование порта сразу после рестарта контейнера  
    int OPT{1};  
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &OPT, sizeof(OPT));  
  
    sockaddr_in address{};  
    address.sin_family = AF_INET;  
    address.sin_addr.s_addr = INADDR_ANY;  
    address.sin_port = htons(PORT);  
  
    if (bind(server_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {  
        std::cerr << std::format("Error binding socket to {}\n", PORT);  
        return 1;  
    }  
    if (listen(server_fd, 10) < 0) {  
        std::cerr << std::format("Error listening for connections\n");  
        return 1;  
    }  
    std::cout << std::format("Listening for connection: {}\n", PORT);  
  
    while (true) {  
        sockaddr_in client_addr{};  
        socklen_t client_len = sizeof(client_addr);  
        int client_fd = accept(server_fd, reinterpret_cast<sockaddr*>(&client_addr), &client_len);  
        if (client_fd < 0) {  
            std::cerr << std::format("Error accepting connection\n");  
            continue;  
        }  
        char buffer[1024] = {0};  
        ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);  
        if (bytes_read > 0) {  
            std::string received{buffer, static_cast<size_t>(bytes_read)};  
            // убираем перевод строки на конце, если он есть  
            while (!received.empty() &&  
                (received.back() == '\r' || received.back() == '\n')) {  
                received.pop_back();  
            }            std::cout << std::format("Received: {}\n", received);  
  
            std::string response = std::format("ECHO: {}\n", received);  
            send(client_fd, response.c_str(), response.length(), 0);  
        }        close(client_fd);  
    }  
    close(server_fd);  
  
    return 0;  
}
```

### consumer/CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.20)  
project(consumer LANGUAGES CXX)  
  
set(CMAKE_CXX_STANDARD 20)  
set(CMAKE_CXX_STANDARD_REQUIRED  ON)  
  
add_executable(consumer main.cpp)
```

### consumer/Dockerfile
```Dockerfile
FROM alpine:3.20 AS builder  
  
RUN apk add --no-cache build-base cmake  
  
WORKDIR /src  
COPY CMakeLists.txt .  
COPY main.cpp .  
  
RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release && \  
    cmake --build build  
  
FROM alpine:3.20  
  
RUN apk add --no-cache libstdc++  
  
WORKDIR /app  
COPY --from=builder /src/build/consumer .  
  
EXPOSE 5000  
ENTRYPOINT ["./consumer"]
```

### producer/main.cpp
#### клиент, шлёт запрос и печатает ответ
```cpp
#include <arpa/inet.h>  
#include <cstring>  
#include <iostream>  
#include <format>  
#include <netdb.h>  
#include <string>  
#include <sys/socket.h>  
#include <unistd.h>  
  
int main(int argc, char *argv[]) {  
    // Хост по умолчанию — "consumer", это ИМЯ контейнера в user-defined bridge сети,  
    // резолвится через встроенный DNS Docker'а    const std::string HOST{argc > 1 ? argv[1] : "consumer"};  
    const std::string MESSAGE{argc > 2 ? argv[2] : "Hello from producer!"};  
    const int PORT{5000};  
  
    addrinfo hints{};  
    hints.ai_family = AF_INET;  
    hints.ai_socktype = SOCK_STREAM;  
  
    addrinfo* result{nullptr};  
    int status{getaddrinfo(  
            HOST.c_str(),  
            std::to_string(PORT).c_str(),  
            &hints,  
            &result  
        )  
    };    if (status != 0) {  
        std::cerr << std::format("Can not resolve host: {}::{}\n", HOST, gai_strerror(status));  
        return 1;  
    }  
    int sock_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);  
    if (sock_fd < 0) {  
        std::cerr << "Can not create socket\n";  
        freeaddrinfo(result);  
        return 1;  
    }  
    if (connect(sock_fd, result->ai_addr, result->ai_addrlen) < 0) {  
        std::cerr << std::format("Can not connect to socket {}:{}\n", HOST, PORT);  
        freeaddrinfo(result);  
        close(sock_fd);  
        return 1;  
    }    freeaddrinfo(result);  
  
    std::cout << std::format("Connected to {}:{}, send: {}\n", HOST, PORT, MESSAGE);  
  
    std::string request{MESSAGE + '\n'};  
    send(sock_fd, request.c_str(), request.size(), 0);  
  
    char buffer[1024] = {0};  
    ssize_t bytes_read = read(sock_fd, buffer, sizeof(buffer) - 1);  
    if (bytes_read > 0) {  
        std::cout << std::format("Answer: {}\n", std::string(buffer, static_cast<size_t>(bytes_read)));  
    }  
    close(sock_fd);  
    return 0;  
}
```

### producer/CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.20)  
project(producer LANGUAGES CXX)  
  
set(CMAKE_CXX_STANDARD 20)  
set(CMAKE_CXX_STANDARD_REQUIRED ON)  
  
add_executable(producer main.cpp)
```

### producer/Dockerfile
```Dockerfile
# ---- этап сборки ----  
FROM alpine:3.20 AS builder  
  
RUN apk add --no-cache build-base cmake  
  
WORKDIR /src  
COPY CMakeLists.txt .  
COPY main.cpp .  
  
RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release && \  
    cmake --build build  
  
# ---- финальный этап ----  
FROM alpine:3.20  
  
RUN apk add --no-cache libstdc++  
  
WORKDIR /app  
COPY --from=builder /src/build/producer .  
  
ENTRYPOINT ["./producer"]
```

## Запуск на практике

```powershell
# 1. Создаём user-defined bridge сеть
docker network create mynet

# 2. Собираем оба образа
docker build -t consumer:1.0 ./consumer
docker build -t producer:1.0 ./producer

# 3. Запускаем consumer в фоне, с именем "consumer" — именно оно резолвится через DNS
docker run -d --name consumer --network mynet consumer:1.0

# 4. Смотрим, что consumer действительно слушает
docker logs consumer

# 5. Запускаем producer — он подключится к consumer ПО ИМЕНИ, не по IP
docker run --rm --network mynet producer:1.0 consumer "Привет, мир!"
```

Ожидаемый вывод producer:

```
Подключились к consumer:5000, отправляем: Привет, мир!
Ответ: ECHO: Привет, мир!
```

И в логах `consumer`:

```powershell
docker logs consumer
```

```
Consumer слушает порт 5000...
Получено: Привет, мир!
```

## Проверка того, что мы разбирали про DNS

Попробуй запустить `producer` **без** сети (или с сетью `bridge` по умолчанию вместо `mynet`):

```powershell
docker run --rm --network bridge producer:1.0 consumer "тест"
```

Получишь `Не удалось резолвить хост consumer` — потому что в дефолтном bridge, как мы разбирали, DNS-резолвинга по имени контейнера не существует. Это наглядно подтверждает разницу между default bridge и user-defined bridge на реальном коде.
