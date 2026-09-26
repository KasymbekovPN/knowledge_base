
# Message Queue — теория

## Что это и чем отличается от pipe/FIFO

Message queue — как и pipe/FIFO, это IPC через ядро с копированием данных, но с ключевым отличием: **сохраняются границы сообщений**. Отправитель кладёт дискретный "пакет" данных, получатель забирает его **целиком**, как единое сообщение — не байтовый поток, где нужно самостоятельно парсить, где кончается одно сообщение и начинается другое.

## POSIX message queue — API

```c
#include <mqueue.h>

mqd_t mq_open(const char* name, int flags, mode_t mode, struct mq_attr* attr);
int mq_send(mqd_t mqd, const char* msg, size_t len, unsigned priority);
ssize_t mq_receive(mqd_t mqd, char* buf, size_t len, unsigned* priority);
int mq_close(mqd_t mqd);
int mq_unlink(const char* name);
```

Как и POSIX shared memory (`shm_open`), очередь идентифицируется **именем** (`"/my_queue"`) — независимые процессы находят её без родства через `fork()`.

## Уникальная фича — приоритеты сообщений

Это то, чего нет ни у pipe/FIFO, ни у обычных сокетов: каждое сообщение отправляется с целочисленным **приоритетом**. `mq_receive` **всегда** возвращает сообщение с **наивысшим** приоритетом среди ожидающих в очереди, а не просто самое старое (FIFO-порядок сохраняется только **внутри** одного уровня приоритета).

## Персистентность и лимиты

Очередь, как и именованная shared memory, **переживает** процесс, который её создал, пока кто-то явно не вызовет `mq_unlink()` (или пока не перезагрузится система — очереди не переживают reboot, в отличие от файлов на диске). У очереди есть **лимиты**: максимальное число сообщений (`mq_maxmsg`) и максимальный размер одного сообщения (`mq_msgsize`) — задаются при создании, `mq_send` **блокируется** (или возвращает `EAGAIN` в неблокирующем режиме), если очередь заполнена.


### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 4.4.2)
project(demo CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(mq_receiver OBJECT mq_receiver.cpp)
add_library(mq_sender OBJECT mq_sender.cpp)

if(LINUX)
    target_link_libraries(mq_receiver PRIVATE pthread rt)
endif ()

```

### mq_sender.cpp
```cpp
#include <mqueue.h>
#include <fcntl.h>
#include <cstring>
#include <iostream>
#include <format>

int main(int argc, char *argv[]) {
    constexpr auto name{"/demo_mq"};

    // struct mq_attr attr{};
    mq_attr attr{};
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 256;

    const mqd_t mq{mq_open(name, O_CREAT | O_WRONLY, 0666, &attr)};
    if (mq == static_cast<mqd_t>(-1)) {
        perror("mq_open");
        return 1;
    }

    struct { const char* text; unsigned priority; } messages[] = {
        {.text = "low priority (usual log)", .priority = 1},
        {.text = "high priority (alert)", .priority = 10},
        {.text = "medium priority (usual event)", .priority = 5},
    };

    for (auto& [text, priority]: messages) {
        std::cout << std::format("[sender] sending: {} (priority={})\n", text, priority) << std::flush;
        if (mq_send(mq, text, strlen(text), priority) != 0) {
            perror("mq_send");
        }
    }

    mq_close(mq);
    std::cout << "[sender] Done\n" << std::flush;

    return 0;
}

```

### mq_receiver.cpp
```cpp
#include <mqueue.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <format>
#include <vector>
#include <chrono>
#include <thread>

int main() {
    constexpr auto name{"/demo_mq"};

    mqd_t mq;
    int attempts{};
    while ((mq = mq_open(name, O_RDONLY)) == static_cast<mqd_t>(-1)) {
        if (++attempts > 50) {
            perror("mq_open");
            return 1;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    // struct mq_attr {};
    mq_attr attr{};
    mq_getattr(mq, &attr);

    std::vector<char> buf(attr.mq_msgsize);
    for (int i{}; i < 3; ++i) {
        unsigned priority{};
        const ssize_t n{mq_receive(mq, buf.data(), buf.size(), &priority)};
        if (n < 0) {
            perror("mq_receive");
            break;
        }
        std::cout << std::format("[receiver] took (priority={}) {}\n",
            priority,
            std::string(buf.data(), n)
        ) << std::flush;
    }

    mq_close(mq);
    mq_unlink(name);

    return 0;
}

```

### Dockerfile
```Dockerfile
# ============================================================
# Сборка:
#   docker build -f Dockerfile -t mq-demo .
#
# Запуск:
#   docker run --rm mq-demo
#
# docker run --rm -it -v "${PWD}:/app" mq-demo bash
#
# ============================================================

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends g++ && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY mq_receiver.cpp mq_sender.cpp run.sh /app/

RUN g++ -std=c++20 -O2 -pthread mq_sender.cpp -o mq_sender -lrt

RUN g++ -std=c++20 -O2 -pthread mq_receiver.cpp -o mq_receiver -lrt

RUN chmod +x /app/run.sh

CMD ["/app/run.sh"]

```

### run.sh
```cpp
#!/bin/sh
set -e

echo "=== launch mq_sender ==="
./mq_sender & MQ_SENDER_PID=$!

#sleep 0.3

echo "=== launch mq_receiver ==="
./mq_receiver

wait "$MQ_SENDER_PID"
echo "=== both processes have been finished, EXIT: $? ==="

```

## Важное отличие от предыдущих примеров — sender может отработать полностью ДО receiver'а

В отличие от FIFO (где `open()` блокируется до появления обеих сторон) и Unix socket (где нужен `connect()`/`accept()`), у message queue **нет** такой точки рандеву — sender отправил все сообщения и завершился, receiver подключился **позже** и всё равно получил их все. Это следствие персистентности очереди — сообщения хранятся в ядре, пока их не заберут, независимо от того, жив ли ещё отправитель. Это делает message queue более похожей по семантике на shared memory (данные переживают процесс) или на message broker (RabbitMQ и т.п.) в миниатюре, чем на pipe/FIFO/сокеты, где обе стороны должны быть "одновременно живы" для передачи данных.

## Сравнение всех разобранных механизмов

|                   | Границы сообщений                       | Приоритеты   | Персистентность после завершения sender'а | Требует одновременной "жизни" обеих сторон |
| ----------------- | --------------------------------------- | ------------ | ----------------------------------------- | ------------------------------------------ |
| Pipe/FIFO         | Нет (байтовый поток)                    | Нет          | Нет                                       | Да (для FIFO — на этапе `open()`)          |
| Unix socket       | Да (`SOCK_DGRAM`) / Нет (`SOCK_STREAM`) | Нет          | Нет                                       | Да                                         |
| **Message queue** | **Да**                                  | **Да**       | **Да**                                    | **Нет**                                    |
| Shared memory     | Не применимо                            | Не применимо | Да (до `shm_unlink`)                      | Нет, но нужна собственная синхронизация    |
