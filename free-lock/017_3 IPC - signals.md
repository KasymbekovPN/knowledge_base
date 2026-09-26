
# Signals — теория

## Что это

Signal — асинхронное уведомление, доставляемое процессу **ядром**, прерывающее нормальный поток выполнения в произвольной точке. В отличие от всех предыдущих IPC-механизмов, сигнал **не переносит содержательные данные** (кроме своего номера и, опционально, одного целого/указателя через real-time сигналы) — это ближе к "прерыванию", чем к "каналу передачи".

## Стандартные vs real-time сигналы

**Стандартные** (`SIGTERM`, `SIGINT`, `SIGUSR1`, `SIGUSR2` и т.д., номера 1-31) — **не ставятся в очередь**: если процесс уже "должен" получить `SIGUSR1`, а ему присылают ещё один `SIGUSR1` до того, как первый обработан, второй **теряется** — оба схлопываются в одно уведомление "у тебя есть SIGUSR1".

**Real-time сигналы** (`SIGRTMIN`...`SIGRTMAX`) — **ставятся в очередь**, ни один не теряется, и что важно — умеют переносить **одно целое или указатель** через `sigqueue()`, в отличие от `kill()`, который отправляет только номер сигнала без данных.

## Async-signal-safety — критическое ограничение

Обработчик сигнала может прерывать программу **в любой точке**, включая середину выполнения `malloc()`, `printf()`, даже середину модификации структур данных вашей же программы. Из-за этого внутри обработчика можно вызывать только небольшой список **async-signal-safe** функций (POSIX явно перечисляет их — в основном "сырые" системные вызовы вроде `write()`, но не `printf()`, не `malloc()`, не большинство C++ стандартной библиотеки). Нарушение этого правила — undefined behavior, которое может годами не проявляться и внезапно дать deadlock или heap corruption в проде.

**Классическое решение** — обработчик только **устанавливает флаг** (`volatile sig_atomic_t` или `std::atomic<bool>` — с оговоркой, что не все атомарные операции формально async-signal-safe, но `store`/`load` для lock-free `atomic<bool>` на практике безопасны), а вся реальная работа выполняется в основном цикле программы, который периодически проверяет флаг.

## `signalfd` — превращение сигнала в обычный файловый дескриптор (Linux)

Это прямая параллель ко всему, что разбирали про epoll: `signalfd()` создаёт fd, из которого можно **читать** структуру с информацией о пришедшем сигнале — вместо асинхронного прерывания в случайной точке кода, сигнал превращается в **обычное событие готовности**, которое можно добавить в тот же `epoll_wait()`, что уже мультиплексирует ваши сокеты. Никаких async-signal-safety ограничений — обработка происходит синхронно, в основном потоке событийного цикла, ровно как обработка данных от клиента в наших epoll-реакторах.

## Пример 1 — классический обработчик + флаг, между двумя независимыми процессами

#### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 4.4.2)
project(demo CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(sig_receiver OBJECT sig_receiver.cpp)
add_library(sig_sender OBJECT sig_sender.cpp)

```

#### sig_sender.cpp
```cpp
#include <csignal>
#include <iostream>
#include <format>
#include <fstream>

int main(int argc, char *argv[]) {
    constexpr auto path{"/tmp/sig_receiver.pid"};
    std::ifstream pidfile(path);
    int target_pid;
    pidfile >> target_pid;
    std::cout << std::format("[SENDER] pid= {}, from= {}\n", target_pid, path) << std::flush;

    kill(target_pid, SIGUSR1);

    return 0;
}

```

#### sig_receiver.cpp
```cpp
#include <csignal>
#include <iostream>
#include <format>
#include <fstream>

namespace {
    volatile sig_atomic_t got_usr1{0};
    volatile sig_atomic_t got_term{0};

    void handler(const int sig) {
        if (sig == SIGUSR1) got_usr1 = 1;
        if (sig == SIGTERM) got_term = 1;
    }
}

int main(int argc, char *argv[]) {
    const auto pid{getpid()};
    std::cout << std::format("[RECEIVER] PID: {}\n", pid) << std::flush;

    constexpr auto path{"/tmp/sig_receiver.pid"};
    std::ofstream pidfile{path};
    pidfile << pid;

    struct sigaction sa{};
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);

    while (!got_term) {
        if (got_usr1) {
            got_usr1 = 0;
            /* processing */
        }
        usleep(10000);
    }

    return 0;
}

```

#### Dockerfile
```Dockerfile
## ============================================================
## Сборка:
##   docker build -f Dockerfile -t sig-classical-demo .
##
## Запуск:
##   docker run --rm sig-classical-demo
##
## docker run --rm -it -v "${PWD}:/app" sig-classical-demo bash
##
## ============================================================

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends g++ && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY sig_receiver.cpp sig_sender.cpp run.sh /app/

RUN g++ -std=c++20 -O2 sig_receiver.cpp -o sig_receiver

RUN g++ -std=c++20 -O2 sig_sender.cpp -o sig_sender

RUN chmod +x /app/run.sh

CMD ["/app/run.sh"]

```

#### run.sh
```shell
#!/bin/sh
set -e

echo "=== launch sig_receiver ==="
./sig_receiver & SIG_RECEIVER=$!

sleep 0.5

echo "=== launch sig_sender ==="
./sig_sender

wait "${SIG_RECEIVER}"
echo "=== both processes have been finished, EXIT: $? ==="

```

### **`signalfd` + epoll подход:**

```cpp
sigset_t mask;
sigemptyset(&mask);
sigaddset(&mask, SIGUSR1);
sigaddset(&mask, SIGTERM);
sigprocmask(SIG_BLOCK, &mask, nullptr);

int sfd = signalfd(-1, &mask, SFD_NONBLOCK);

int epoll_fd = epoll_create1(0);
epoll_event ev{};
ev.events = EPOLLIN;
ev.data.fd = sfd;
epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sfd, &ev);

// в основном цикле:
signalfd_siginfo info{};
read(sfd, &info, sizeof(info));
```

## Сравнение двух подходов

| |Классический (`sigaction` + флаг)|`signalfd` + epoll|
|---|---|---|
|Портируемость|POSIX, работает везде|Linux-специфично|
|Async-signal-safety|Нужно строго соблюдать в обработчике|Не актуально — обработка синхронная|
|Интеграция с event loop|Нужен polling-цикл с флагом отдельно от epoll|**Естественно** встраивается в существующий `epoll_wait`|
|Задержка реакции|Ограничена частотой polling-цикла (`usleep`)|Точная — как обычное событие epoll|
|Потеря сигналов|Возможна для стандартных сигналов при "залповой" отправке|То же самое (`signalfd` не даёт real-time семантику сама по себе — для неё нужны именно real-time сигналы `SIGRTMIN+N`)|

## Прямая связь с архитектурой из Блока 3

Если бы наш epoll-реактор из ранних демок должен был реагировать на `SIGTERM` для graceful shutdown (вместо `poll()` с 100мс таймаутом, который мы использовали для periodic-проверки `running`) — `signalfd` дал бы **более чистое** решение: добавить `sfd` в тот же `epoll_fd`, что уже слушает клиентские сокеты, и обрабатывать shutdown-сигнал **тем же кодом**, что обрабатывает готовность клиентов — без отдельного механизма опроса флага, без произвольного таймаута "как часто проверять" — просто ещё одно событие в том же цикле.

## Резюме серии IPC

Мы прошли всю таксономию, с которой начали: pipe/FIFO (байтовый поток, требует родства или общего пути), Unix domain socket (полнодуплексный, connection-oriented, единственный с передачей fd), message queue (дискретные сообщения с приоритетами, персистентные), shared memory (без копирования, но без встроенной синхронизации), и signals (асинхронное уведомление без содержательных данных, либо real-time с одним значением). Каждый — свой компромисс между простотой, производительностью, структурированностью данных и требованиями к синхронизации, ровно как обсуждали в самом первом ответе серии.

# Разбор `kill(target_pid, SIGUSR1)`

## `SIGUSR1` — откуда берётся

Это **предопределённая константа** из системного заголовка:

```cpp
#include <csignal>   // C++ обёртка над <signal.h>
// или
#include <signal.h>  // C-заголовок напрямую
```

`SIGUSR1` (и `SIGUSR2`) — **единственные** два стандартных сигнала POSIX, у которых **нет** зафиксированного системой значения ("определяемые пользователем"). Все остальные сигналы (`SIGTERM`, `SIGINT`, `SIGKILL`, `SIGSEGV` и т.д.) имеют **предустановленный смысл** и часто предустановленное поведение по умолчанию (например, `SIGSEGV` по умолчанию завершает процесс с core dump) — `SIGUSR1`/`SIGUSR2` же существуют **специально** для того, чтобы приложения сами придумывали им значение, ровно как в нашем примере ("получен сигнал — увеличь счётчик").

Числовое значение `SIGUSR1` **платформозависимо** (на Linux x86 — обычно `10`, на других архитектурах/ОС может отличаться) — именно поэтому в коде **всегда** используют символическую константу `SIGUSR1`, а не магическое число `10`.

## `target_pid` — откуда берётся PID

Это **process ID** — число, которое ядро присваивает **каждому** запущенному процессу при его создании. В нашем примере он был получен через файл-рандеву:

```cpp
// В receiver:
std::ofstream pidfile("/tmp/sig_receiver.pid");
pidfile << getpid();   // <-- getpid() возвращает PID ЭТОГО процесса
```

```cpp
// В sender:
std::ifstream pidfile("/tmp/sig_receiver.pid");
pidfile >> target_pid;  // <-- читаем число из файла
```

## Другие способы узнать PID нужного процесса — в реальных сценариях

Файл с PID (как в нашем демо) — один из вариантов, но не единственный и не всегда лучший. Вот более общая картина:

**1. Родственный процесс — `fork()` возвращает PID напрямую:**

```cpp
pid_t child_pid = fork();
if (child_pid > 0) {
    // мы родитель, child_pid -- PID только что созданного потомка
    kill(child_pid, SIGUSR1);
}
```

**2. Найти процесс по имени — через `pgrep`/`/proc`:**

```bash
pgrep -f "my_server"      # выведет PID процессов с этим именем в командной строке
kill -USR1 $(pgrep my_server)
```

Или программно — сканирование `/proc/*/comm` или `/proc/*/cmdline` в поисках нужного имени процесса.

**3. PID-файл, который сам процесс пишет при старте** — это ровно наш подход, только обычно кладут в стандартное место типа `/var/run/myapp.pid`, а не в `/tmp` — стандартная практика для демонов/сервисов на Linux (многие init-системы, включая systemd, читают именно такой файл, чтобы знать, кому слать `SIGTERM` при остановке сервиса).

**4. `kill -l` в шелле** — если хотите вручную отправить сигнал уже запущенному процессу для теста:

```bash
ps aux | grep my_program    # найти PID глазами
kill -USR1 12345             # 12345 -- PID из вывода ps
```

## Специальные значения `target_pid`, которые стоит знать

`kill()` — не только "отправь сигнал конкретному процессу", у аргумента `pid` есть особые значения:

|Значение `pid`|Что означает|
|---|---|
|`> 0`|Отправить конкретному процессу с этим PID (наш случай)|
|`0`|Отправить **всем** процессам в той же process group, что и вызывающий|
|`-1`|Отправить **всем** процессам, на которые у вызывающего есть права (кроме init)|
|`< -1` (например, `-500`)|Отправить всем процессам в group с ID `500`|

## Практический риск в вашем коде — что если PID переиспользован

Важная деталь, которую в демо-коде мы не проверяли: PID'ы **переиспользуются** ОС (та же логика, что разбирали для файловых дескрипторов раньше в этом разговоре — "наименьший свободный номер"). Если receiver из вашего примера **успел завершиться и его PID подхватил совершенно другой процесс**, прежде чем sender прочитал устаревший PID-файл — `kill()` отправит сигнал **не тому** процессу. Реальные системы обычно решают это либо явной проверкой "жив ли процесс с ожидаемым именем" (через `/proc/<pid>/comm`), либо просто принятием риска как маловероятного edge case в контролируемых средах (что и было сделано в нашем демо ради простоты).
