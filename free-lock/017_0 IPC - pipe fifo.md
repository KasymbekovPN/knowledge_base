

# Pipe / FIFO — теория

## Anonymous pipe — только между родственными процессами

```c
int pipefd[2];
pipe(pipefd);
// pipefd[0] -- конец на ЧТЕНИЕ
// pipefd[1] -- конец на ЗАПИСЬ
```

`pipe()` создаёт **однонаправленный** байтовый канал внутри ядра и возвращает два файловых дескриптора на его концы. Ключевое ограничение: у анонимного pipe **нет имени** в файловой системе — единственный способ, которым второй процесс может получить доступ к этим же fd, это **унаследовать** их через `fork()` (таблица fd копируется в потомка). Отсюда и название "родственный" IPC — работает только между процессом и его потомками.

**Типичная схема "закрыть неиспользуемый конец":**

```
До fork():           После fork(), родитель:      После fork(), потомок:
pipefd[0] (read)     закрывает pipefd[1]           закрывает pipefd[0]
pipefd[1] (write)    (не пишет сам)                 (не читает сам)
```

Закрытие неиспользуемого конца — не просто гигиена: если родитель, читающий из pipe, **не закроет** свою копию write-конца, `read()` никогда не увидит EOF (ядро считает канал открытым для записи, пока хоть один процесс держит write-конец открытым), даже если потомок, который реально пишет, уже завершился.

## Гарантия атомарности — `PIPE_BUF`

POSIX гарантирует: запись размером **до `PIPE_BUF`** байт (обычно 4096 на Linux) атомарна — то есть если несколько писателей пишут одновременно в один pipe, их записи **не перемешаются** побайтово, каждая `write()` целиком попадёт в канал как единый блок. Записи **больше** `PIPE_BUF` такой гарантии не имеют — могут чередоваться с записями других писателей.

## Блокирующее поведение

- `read()` на пустом pipe **блокируется**, пока не появятся данные или все write-концы не закроются (тогда возвращает `0` — EOF).
- `write()` на заполненный pipe (буфер ядра, обычно 64KB на Linux) **блокируется**, пока читатель не освободит место.
- `O_NONBLOCK` меняет это на `EAGAIN` вместо блокировки — тот же принцип, что разбирали для сокетов.

## FIFO (named pipe) — та же семантика, но с именем

```c
mkfifo("/tmp/my_fifo", 0666);   // создаёт специальный файл в файловой системе
```

FIFO — **тот же** механизм внутри ядра (тот же круговой буфер, те же гарантии `PIPE_BUF`, то же блокирующее поведение), но с одним отличием: у него есть **путь** в файловой системе, через который **любые независимые** процессы (не обязательно родственные) находят один и тот же канал через обычный `open()`.

**Важная тонкость: `open()` на FIFO по умолчанию блокируется**, пока не откроется **противоположный** конец — `open()` на чтение ждёт, пока кто-то не откроет на запись, и наоборот. Это встроенная точка рандеву — удобно для простой синхронизации "подождать, пока оба участника готовы", но неожиданно для тех, кто не в курсе.

### Anon pipe

**`anon_pipe.cpp`** (fork-based, работает):

```cpp
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <iostream>
#include <format>
#include <string>
#include <cstdlib>
#include <thread>
#include <chrono>

namespace {
    template <typename... Args>
    void print_log(const int pid, std::format_string<Args...> fmt, Args&&... args) {
        std::cout << "[" << pid << "] " << std::format(fmt, std::forward<Args>(args)...) << std::flush;
    }
}

int main(int argc, char *argv[]) {
    constexpr int TIMEOUT_S{5};
    std::this_thread::sleep_for(std::chrono::seconds(TIMEOUT_S));

    int pipe_fd[2];
    if (pipe(pipe_fd) != 0) {
        perror("pipe");
        return 1;
    }

    const pid_t pid{fork()};
    print_log(pid, "PID: {}\n", pid);
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        close(pipe_fd[0]);
        constexpr auto msg{"hello from child"};
        write(pipe_fd[1], msg, strlen(msg));
        close(pipe_fd[1]);

        std::this_thread::sleep_for(std::chrono::seconds(TIMEOUT_S));

        _exit(0);
    }

    close(pipe_fd[1]);
    char buf[256] = {};
    const ssize_t n{read(pipe_fd[0], buf, sizeof(buf) - 1)};
    // std::cout << std::format("[parent] read {} bytes: {}\n", n , buf) << std::flush ;
    print_log(pid, "[parent] read {} bytes: {}\n", n, buf);
    close(pipe_fd[0]);
    waitpid(pid, nullptr, 0);

    std::this_thread::sleep_for(std::chrono::seconds(TIMEOUT_S));

    return 0;
}

```

### FIFO

#### Dockerfile-fifo

```Dockerfile
# ============================================================
# Dockerfile.fifo -- сборка и запуск демо FIFO
# (fifo_writer.cpp / fifo_reader.cpp).
#
# Сборка:
#   docker build -f Dockerfile-fifo -t fifo-demo .
#
# Запуск:
#   docker run --rm fifo-demo
#
# docker run --rm -it -v "${PWD}:/app" fifo-demo bash
#
# ============================================================

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends g++ && rm -rf /var/lib/apt/lists/* \

WORKDIR /app

COPY fifo_writer.cpp fifo_reader.cpp run.sh /app/

RUN g++ -std=c++20 -O2 /app/fifo_writer.cpp -o fifo_writer

RUN g++ -std=c++20 -O2 /app/fifo_reader.cpp -o fifo_reader

RUN chmod +x /app/run.sh

CMD ["/app/run.sh"]

```

#### run.sh

```shell
#!/bin/sh
set -e
FIFO_PATH=/tmp/demo_fifo

rm -f "${FIFO_PATH}"
mkfifo "${FIFO_PATH}"

echo "=== FIFO has been created, launch the reader in the background ==="
./fifo_reader & READER_PID=$!

sleep 0.3

echo "=== launch writer ==="
./fifo_writer

wait "$READER_PID"
echo "=== both processes have been finished, EXIT: $? ==="

```

#### file_writer.cpp
```cpp
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <iostream>
#include <format>

int main(int argc, char *argv[]) {
    constexpr auto path{"/tmp/demo_fifo"};
    mkfifo(path, 0666);

    std::cout << "[writer] FIFO opened on write...\n";
    const int fd{open(path, O_WRONLY)};
    std::cout << "[writer] reader in online, write data...\n";

    for (int i{1}; i <= 5; ++i) {
        std::string msg{std::format("message #{}\n", i)};
        write(fd, msg.data(), msg.size());
    }

    close(fd);
    std::cout << "[writer] Done\n";

    return 0;
}

```

#### fifo_reader.cpp
```cpp
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <format>

int main(int argc, char *argv[]) {
    constexpr auto path{"/tmp/demo_fifo"};

    std::cout << "[reader] open FIFO on read...\n";
    const int fd{open(path, O_RDONLY)};
    if (fd < 0) {
        perror("[reader] open failed");
        return 1;
    }
    std::cout << "[reader] writer is online, read data\n";

    char buf[256];
    ssize_t n;
    while ((n = read(fd, buf, sizeof(buf) - 1)) > 0) {
        buf[n] = '\0';
        std::cout << std::format("[reader] took: {}\n", buf);
        // std::cout << "[reader] took: " << buf << "\n";
    }
    std::cout << "[reader] EOF --- writer have closed connection\n";

    close(fd);
    unlink(path);

    return 0;
}

```


**Запуск** (обязательно `mkfifo` до старта любого из процессов):

```bash
mkfifo /tmp/demo_fifo
./fifo_reader &
./fifo_writer
```
