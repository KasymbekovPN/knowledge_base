
# Unix Domain Socket — теория

## Что это

Unix domain socket (UDS) — IPC-механизm, использующий тот же **socket API** (`socket()`, `bind()`, `listen()`, `accept()`, `connect()`, `send()`/`recv()`), что и TCP/IP-сокеты, но работающий **только локально**, внутри одной машины, без сетевого стека вообще. Семейство адресов — `AF_UNIX` (синоним `AF_LOCAL`), вместо IP:порт адресация идёт через **путь в файловой системе**.

```c
struct sockaddr_un {
    sa_family_t sun_family;   // AF_UNIX
    char sun_path[108];       // путь, например "/tmp/my_socket"
};
```

## Почему это не то же самое, что TCP через loopback

Хотя API идентичен коду, который мы писали для TCP-серверов (epoll-реактор, thread-per-connection) ранее в этом разговоре, UDS **не проходит** через сетевой стек ядра — нет IP-заголовков, нет TCP checksums, нет congestion control, нет sequence numbers. Данные копируются напрямую между буферами процессов через внутреннюю структуру ядра. Это ощутимо быстрее TCP loopback при высокой частоте небольших сообщений — типичная разница в 2-4 раза по throughput на практике.

## Stream vs Datagram

Как и в обычных сокетах:

- **`SOCK_STREAM`** — байтовый поток, сохраняющий порядок, без границ сообщений (как TCP) — то, что использовали в epoll-демках.
- **`SOCK_DGRAM`** — сохраняет **границы сообщений** (как UDP), но, в отличие от настоящего UDP, **надёжен** — сообщения не теряются и не переупорядочиваются (потому что нет реальной сети, только локальная буферизация ядра).

## Уникальная фича — передача файловых дескрипторов между процессами

Это то, чего **нет** ни у pipe, ни у shared memory, ни у обычных TCP-сокетов: через специальный тип сообщения `SCM_RIGHTS` (передаётся в ancillary data `sendmsg()`/`recvmsg()`) можно **передать открытый файловый дескриптор** одного процесса другому. Получатель получает **новую** запись в своей собственной таблице fd, указывающую на **ту же** open file description — классическое применение: главный процесс открывает привилегированный ресурс (например, слушающий сокет на порту < 1024) и передаёт его непривилегированным worker-процессам.

## Именование — path-based vs abstract namespace (только Linux)

- **Path-based** (`/tmp/my_socket`) — создаёт реальный файл-специальный узел в файловой системе. Нужно вручную удалять (`unlink`) перед повторным использованием — та же проблема, что мы ловили с FIFO (`bind()` провалится с `EADDRINUSE`, если файл остался от предыдущего запуска).
- **Abstract namespace** (Linux-специфично) — путь начинается с `\0` (нулевого байта), не создаёт файла в файловой системе вообще, автоматически исчезает, когда сокет закрывается. Удобнее для тестов/демок именно потому, что не оставляет "мусора" на диске.

## Итоговый код

#### Dockerfile
```Dockerfile
# ============================================================
# Сборка:
#   docker build -f Dockerfile -t uds-demo .
#
# Запуск:
#   docker run --rm uds-demo
#
# docker run --rm -it -v "${PWD}:/app" uds-demo bash
#
# ============================================================

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends g++ && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY framing.hpp uds_client.cpp uds_server.cpp run.sh /app/

RUN g++ -std=c++20 -O2 -pthread uds_server.cpp -o uds_server

RUN g++ -std=c++20 -O2 -pthread uds_client.cpp -o uds_client

RUN chmod +x /app/run.sh

CMD ["/app/run.sh"]

```

#### framing.hpp
```cpp
#pragma once

// Framing с префиксом длины: [uint32 длина в network byte order][payload].
// SOCK_STREAM не хранит границы сообщений, поэтому их задаём сами.

#include <arpa/inet.h>
#include <unistd.h>
#include <cerrno>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

inline constexpr std::uint32_t max_msg_size{64 * 1024};

// write может записать меньше, чем просили, дописываем в цикле.
inline bool write_all(const int fd, const void* data, std::size_t size) {
    auto p{static_cast<const char*>(data)};
    while (size > 0) {
        const ssize_t n{write(fd, p, size)};
        if (n < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        p += n;
        size -= static_cast<std::size_t>(n);
    }
    return true;
}

// read может вернуть часть данных, дочитываем ровно size байт.
// false: EOF или ошибка.
inline bool read_all(const int fd, void* data, std::size_t size) {
    auto p{static_cast<char*>(data)};
    while (size > 0) {
        const ssize_t n{read(fd, p, size)};
        if (n < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        if (n == 0) return false;
        p += n;
        size -= static_cast<std::size_t>(n);
    }
    return true;
}

inline bool send_msg(const int fd, const std::string_view msg) {
    if (msg.size() > max_msg_size) return false;
    const std::uint32_t len{htonl(static_cast<std::uint32_t>(msg.size()))};
    return write_all(fd, &len, sizeof(len)) && write_all(fd, msg.data(), msg.size());
}

// nullopt: соединение закрыто, ошибка или слишком длинное сообщение.
inline std::optional<std::string> recv_msg(const int fd) {
    std::uint32_t len{};
    if (!read_all(fd, &len, sizeof(len))) return std::nullopt;
    len = ntohl(len);
    if (len > max_msg_size) return std::nullopt;

    std::string msg(len, '\0');
    if (!read_all(fd, msg.data(), len)) return std::nullopt;
    return msg;
}

```

#### uds_server.cpp
```cpp
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <iostream>
#include <format>
#include <cstring>

#include "framing.hpp"

int main() {
    constexpr auto path{"/tmp/demo_uds.sock"};
    unlink(path);

    const int listen_fd{socket(AF_UNIX, SOCK_STREAM, 0)};
    if (listen_fd < 0) {
        perror("socket");
        return 1;
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    if (bind(listen_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        perror("bind");
        return 1;
    }
    if (listen(listen_fd, 5) != 0) {
        perror("listen");
        return 1;
    }
    std::cout << std::format("[server] listen to '{}'\n", path) << std::flush;

    const int client_fd{accept(listen_fd, nullptr, nullptr)};
    if (client_fd < 0 ) {
        perror("accept");
        return 1;
    }
    std::cout << "[server] client has been connected\n" << std::flush;

    while (const auto msg{recv_msg(client_fd)}) {
        std::cout << std::format("[server] took: {}\n",*msg) << std::flush;
        if (!send_msg(client_fd, std::format("echo: {}", *msg))) {
            perror("send_msg");
            break;
        }
    }

    std::cout << "[server] client disconnected\n" << std::flush;
    close(client_fd);
    close(listen_fd);
    unlink(path);

    return 0;
}

```

#### uds_client.cpp
```cpp
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <format>
#include <thread>
#include <chrono>

#include "framing.hpp"

int main() {
    constexpr auto path{"/tmp/demo_uds.sock"};

    const int fd{socket(AF_UNIX, SOCK_STREAM, 0)};
    if (fd < 0) {
        perror("socket");
        return 1;
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    int attempts{0};
    while (connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        if (++attempts > 50) {
            perror("connect");
            return 1;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    std::cout << "[client] connect to server\n" << std::flush;

    for (int i{1}; i <= 3; ++i) {
        if (!send_msg(fd, std::format("hello: #{}", i))) {
            perror("send_msg");
            return 1;
        }

        const auto answer{recv_msg(fd)};
        if (!answer) {
            std::cerr << "[client] server closed connection\n";
            return 1;
        }
        std::cout << std::format("[client] answer from server: {}\n",*answer) << std::flush;
    }

    close(fd);
    std::cout << "[client] done, close connection\n" << std::flush;

    return 0;
}

```

### run.sh
```shell
#!/bin/sh
set -e
UDS_PATH=/tmp/demo_uds.sock

rm -f "${UDS_PATH}"

echo "=== launch server ==="
./uds_server & SERVER_PID=$!

sleep 0.3

echo "=== launch client ==="
./uds_client

wait "${SERVER_PID}"
echo "=== both processes have been finished, EXIT: $? ==="

```

## Урок из FIFO-разбора применён здесь по-другому

С FIFO мы решали проблему упорядочивания через **обязательный порядок вызовов** (`mkfifo` строго до запуска обоих процессов) — там `open()` на несуществующем файле сразу проваливается с ошибкой, а не ждёт. С Unix domain socket то же самое можно решить **элегантнее** — через **retry-цикл на `connect()`**: если сервер ещё не успел `bind()`+`listen()`, `connect()` просто провалится с `ECONNREFUSED`, и клиент со спокойной совестью пробует снова через 20мс. Это устойчивее к порядку запуска — не нужно гарантировать "сервер стартовал первым", достаточно, чтобы клиент был терпелив. Тот же приём стоило бы применить и в TCP-клиентах из более ранних epoll-демок вместо жёсткого `sleep_for(100ms)` перед запуском тестовых клиентов — там мы полагались на удачу с таймингом, а не на явную устойчивость к порядку.

## Сравнение с уже разобранными механизмами

|                                      | Pipe/FIFO                  | Unix domain socket          | Shared memory           |
| ------------------------------------ | -------------------------- | --------------------------- | ----------------------- |
| Направленность                       | Однонаправленный           | Полнодуплексный             | Не применимо — не поток |
| Требует родства                      | Pipe — да, FIFO — нет      | Нет                         | Нет (для POSIX shm)     |
| Передача fd между процессами         | Нет                        | **Да** (`SCM_RIGHTS`)       | Нет                     |
| Модель                               | Байтовый поток / сообщения | Байтовый поток / датаграммы | Прямой доступ к памяти  |
| Копирование через ядро               | Да                         | Да                          | Нет                     |
| Connection-oriented (accept/connect) | Нет                        | Да (для `SOCK_STREAM`)      | Нет                     |
