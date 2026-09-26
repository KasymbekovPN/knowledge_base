

# Shared Memory — теория

## Что делает shared memory принципиально другим механизмом

В отличие от всех предыдущих IPC-механизмов серии (pipe/FIFO, Unix socket, message queue, signals) — здесь **нет копирования данных через ядро на каждую операцию**. Несколько процессов отображают **один и тот же** физический участок памяти на свои (разные!) виртуальные адресные пространства через `mmap()`. Чтение/запись — обычные операции с памятью, без единого системного вызова на каждое обращение.

## `MAP_SHARED` — ключевой флаг

```c
void* mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset);
```

- **`MAP_SHARED`** — записи видны **всем** процессам, отобразившим тот же объект. Это то, что нужно для IPC.
- **`MAP_PRIVATE`** — copy-on-write, записи приватны для каждого процесса (используется для загрузки исполняемых файлов, не для IPC).

## POSIX shared memory через `shm_open`

```c
int fd = shm_open("/my_segment", O_CREAT | O_RDWR, 0666);
ftruncate(fd, size);
void* ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
```

Именованный объект в `tmpfs` (виден в `/dev/shm/`) — независимые процессы находят его по **имени**, ровно как с message queue (`mq_open`) и FIFO (`mkfifo`) из этой же серии.

## Цена за отсутствие копирования — нет встроенной синхронизации вообще

Ядро **не знает и не следит** за тем, что происходит внутри разделяемого региона. Это единственный механизм из всей серии, где нужно **самостоятельно** обеспечивать:

- **Атомарность** операций (обычный `std::mutex` **не работает** между процессами без явного `PTHREAD_PROCESS_SHARED`-атрибута — разбирали это отдельно ранее).
- **Видимость** изменений между процессами — здесь на помощь приходит то же, что использовали весь этот разговор для потоков: `std::atomic` **валиден** в shared memory между процессами (при условии `is_always_lock_free`), потому что atomic-инструкции работают на уровне кэш-когерентности физической памяти, а не привязаны к конкретному процессу.
- **Отсутствие указателей внутри структуры** — единственный указатель, который что-то значит в shared memory, это относительное смещение внутри самого сегмента; обычный `T*`, полученный через `new`, будет валиден только в адресном пространстве процесса, который его создал (виртуальный адрес — не абсолютная истина, разбирали это на примере с `offset_ptr`).

## Пример — переиспользуем нашу SPSC-очередь из lock-free серии

Ключевая деталь, которая делает это возможным без единой правки к коду очереди: буфер хранится **встроенным массивом** (`T buffer_[Capacity]`), а не указателем — внутри структуры физически нет ничего, что ссылалось бы на "внешнюю" по отношению к ней память.Файлы сохранились с прошлого разбора. Пересоберу и перезапущу свежо, чтобы подтвердить, что всё ещё работает, прежде чем показать финальный код:Работает: `1..10` дошли по порядку, разные виртуальные адреса у двух процессов (`0x7f888b...` vs `0x7f749b...`), `EXIT: 0`. Предупреждение компилятора — ровно то самое ABI-нестабильное предупреждение про `hardware_destructive_interference_size`, что разбирали раньше, и здесь оно **особенно уместно**: writer и reader должны быть собраны одинаковым компилятором/флагами.



---

## Итоговый код

**`shm_spsc.h`** (общий заголовок):

```cpp
#pragma once
#include <atomic>
#include <new>

template<typename T, size_t Capacity>
class SPSCQueue {
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be power of 2");
    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> write_pos_{0};
    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> read_pos_{0};
    T buffer_[Capacity]{};   // встроенный массив -- ключевое свойство для shared memory

public:
    bool push(T value) {
        size_t w = write_pos_.load(std::memory_order_relaxed);
        size_t r = read_pos_.load(std::memory_order_acquire);
        if (w - r >= Capacity) return false;
        buffer_[w & (Capacity - 1)] = value;
        write_pos_.store(w + 1, std::memory_order_release);
        return true;
    }

    bool pop(T& result) {
        size_t r = read_pos_.load(std::memory_order_relaxed);
        size_t w = write_pos_.load(std::memory_order_acquire);
        if (r == w) return false;
        result = buffer_[r & (Capacity - 1)];
        read_pos_.store(r + 1, std::memory_order_release);
        return true;
    }
};
```

**`shm_writer.cpp`:**

```cpp
#include "shm_spsc.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

using Queue = SPSCQueue<int, 1024>;

int main() {
    const char* name = "/spsc_ipc_demo";
    int fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (fd < 0) { perror("shm_open"); return 1; }

    ftruncate(fd, sizeof(Queue));
    void* addr = mmap(nullptr, sizeof(Queue), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) { perror("mmap"); return 1; }

    // placement new -- конструируем объект ПРЯМО в общей памяти,
    // только ОДИН процесс должен это делать
    auto* queue = new (addr) Queue();

    std::cout << "[writer] mmap addr в этом процессе = " << addr << "\n";
    for (int i = 1; i <= 10; ++i) {
        while (!queue->push(i)) {}
    }
    std::cout << "[writer] готово\n";

    munmap(addr, sizeof(Queue));
    close(fd);
    return 0;
}
```

**`shm_reader.cpp`:**

```cpp
#include "shm_spsc.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

using Queue = SPSCQueue<int, 1024>;

int main() {
    const char* name = "/spsc_ipc_demo";
    int fd = shm_open(name, O_RDWR, 0666);
    if (fd < 0) { perror("shm_open"); return 1; }

    void* addr = mmap(nullptr, sizeof(Queue), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) { perror("mmap"); return 1; }

    // НЕ вызываем placement new -- объект уже создан writer'ом,
    // просто переинтерпретируем те же байты в своём адресном пространстве
    auto* queue = reinterpret_cast<Queue*>(addr);

    std::cout << "[reader] mmap addr в этом процессе = " << addr << "\n";
    int value, count = 0;
    while (count < 10) {
        if (queue->pop(value)) {
            std::cout << "[reader] получено: " << value << "\n";
            ++count;
        }
    }

    munmap(addr, sizeof(Queue));
    close(fd);
    shm_unlink(name);
    return 0;
}
```

**Сборка и запуск:**

```bash
g++ -std=c++20 -O2 shm_writer.cpp -o shm_writer -lrt
g++ -std=c++20 -O2 shm_reader.cpp -o shm_reader -lrt

./shm_writer &
./shm_reader
```

## Финальное сравнение всей серии IPC

| |Копирование через ядро|Границы сообщений|Встроенная синхронизация|Персистентность|
|---|---|---|---|---|
|Pipe/FIFO|Да|Нет|Да (блокировка read/write)|Нет|
|Unix domain socket|Да|Опционально (`SOCK_DGRAM`)|Да|Нет|
|Message queue|Да|Да, с приоритетами|Да|Да, до `mq_unlink`|
|**Shared memory**|**Нет**|Не применимо|**Нет — сами строите**|Да, до `shm_unlink`|
|Signals|Нет (только номер)|Да (одно уведомление)|Не применимо|Нет|

Shared memory — единственный механизм в серии, требующий от разработчика того же набора знаний, что мы выстраивали весь этот долгий разговор про lock-free структуры: atomic-операции, memory ordering, отсутствие "чужих" указателей внутри общих данных. Это прямая причина, почему вся серия IPC логично продолжила тему lock-free программирования — на границе между процессами все те же законы физики памяти действуют, просто без потоков одного адресного пространства, скрывающих разницу между "мой указатель" и "чужой указатель".


---
---
---
---

## Блок 1 — Фундамент: memory model и atomics (3-5 дней)

**Теория:**

- [x] `std::memory_order`: relaxed, acquire, release, acq_rel, seq_cst — что каждый реально гарантирует на уровне happens-before (2026.09.14)
- [x] Разница между atomicity и ordering  (2026.09.14)
- [x] ABA problem и способы борьбы (tagged pointers, hazard pointers, epoch-based reclamation)  (2026.09.15)
- [x] False sharing и cache line padding (`alignas(64)`, `std::hardware_destructive_interference_size`)  (2026.09.15)
- [x] CAS (compare_exchange_weak vs strong) — почему weak предпочтителен в циклах (2026.09.15)

**Практика:**

- [x] Написать spinlock на atomic_flag, замерить под контейнером с perf (2026.09.17)
- [x] Реализовать простой atomic counter с разными memory_order и посмотреть разницу в асме (godbolt) (2026.09.17)
- [x] Разобрать пример ABA на указателях вручную (2026.09.18)

**Источники:** "C++ Concurrency in Action" (Anthony Williams) главы 5, 7; CppCon talks Herb Sutter "atomic<> Weapons"; cppreference по memory_order.

## Блок 2 — Lock-free структуры данных (1-2 недели)

**Порядок изучения (от простого к сложному):**

1. [x] **MPSC queue** (Michael-Scott или на основе intrusive linked list) — уже ближе к реальному broadcast-паттерну (2026.09.18)
2. [x] **MPMC queue** (Dmitry Vyukov's bounded queue — классика, часто спрашивают на собесах) (2026.09.18)
3. [x] Lock-free stack (Treiber stack) — проще MPMC, хорошо иллюстрирует ABA (2026.09.18)

**Для каждой структуры:**

- [x] Реализовать самому на raw atomics (не подглядывая) (2026.09.21)
- [x] Написать stress-test с несколькими потоками + TSan (ThreadSanitizer) — это критично, lock-free код без санитайзера почти невозможно верифицировать (2026.09.21)
- [x] Сравнить throughput/latency с mutex-based аналогом (тот же интерфейс, `std::mutex + std::deque`) (2026.09.21)

**Источники:** Dmitry Vyukov's blog (1024cores.net) — обязательно; "The Art of Multiprocessor Programming" (Herlihy & Shavit) главы про queues/stacks; boost::lockfree как референс-реализация для сверки.

## Блок 3 — Архитектура lock-free чат-сервера (1 неделя)

**Ключевые решения для чата конкретно:**

- [x] **Модель потоков**: thread-per-connection vs event loop (epoll/io_uring) + пул воркеров — lock-free структуры чаще всего нужны именно во второй модели, как shared-state между воркерами (2026.09.24)
- [x] fd win  vs linux (2026.09.24)
- [x] **Message queue на клиента**: SPSC от воркера к connection-writer (один producer — логика чата, один consumer — сетевой поток на отправку) (2026.09.24)
- [ ] ipc & mmap теория и примеры
- [ ] **Broadcast**: тут MPSC/MPMC не всегда лучший выбор — часто эффективнее copy-on-write список подписчиков (RCU-style) + каждому подписчику своя SPSC-очередь, чем один общий lock-free broadcast-канал
- [ ] **Backpressure**: что делать, если consumer медленнее producer — bounded queue + drop policy vs unbounded + memory blowup
- [ ] **Reclamation проблема**: если пользователь отключается, когда безопасно освободить его структуры при lock-free доступе (hazard pointers / epoch-based / RCU)

**Практика:** собрать минимальный прототип — N потоков-писателей кладут сообщения в шину, M потоков-читателей (эмулируют коннекты) их разбирают, с метриками latency/throughput под нагрузкой.

**Источники:** статьи про архитектуру Discord/Slack backend (общие принципы, не lock-free специфика, но полезно для контекста), RCU paper (Paul McKenney), доклады про io_uring + lock-free от CppCon/Meeting C++.

----
---
---
---
---
---
---
---
---

### **Broadcast**: тут MPSC/MPMC не всегда лучший выбор — часто эффективнее copy-on-write список подписчиков (RCU-style) + каждому подписчику своя SPSC-очередь, чем один общий lock-free broadcast-канал


Реализую broadcast-канал на copy-on-write списке подписчиков (RCU-style read) — каждый подписчик получает сообщение через свою SPSC-очередь, без единого общего lock-free канала.Good — процесс не завис. Причина зависания найдена: bounded SPSC-очередь (capacity 1024) переполняется под 80 000 сообщений, `publish()` тихо дропает лишние, а consumer ждёт получить ровно `TOTAL_MESSAGES` — которого никогда не наберётся. Исправляю: `publish()` делает retry вместо тихого дропа (backpressure, как в предыдущих демках), плюс отдельно уберу лишний `|| true`.Важный момент, который стоит проговорить явно: в моём первом варианте теста 4 publisher-потока **одновременно** писали в один и тот же `inbox` подписчика — а `SPSCQueue` рассчитан ровно на одного producer'а. С несколькими параллельными publisher'ами это уже реальная гонка (multiple producers на "single producer" структуру), а не гипотетическая. Исправляю тест на архитектурно корректный: **один поток владеет топиком** и публикует последовательно — это и есть условие, при котором `SPSC` на подписчика вообще имеет право быть SPSC.Теперь пересоберу с таймаутом на выполнение, чтобы избежать повторного зависания при наличии остаточного бага.Нашёл настоящую причину зависания — это важный практический урок, а не просто баг демки. Поправлю архитектурно правильно.Оба теста прошли: тест 1 — точная доставка 80 000/80 000 всем 5 подписчикам, тест 2 — 165 201 публикация под непрерывным churn (216 725 subscribe/unsubscribe) без единого зависания. Один drop в логе — ровно ожидаемое поведение bounded-retry policy на пограничном случае. Проверю на TSan:TSan чист. Теперь уберу отладочный маркер и покажу финальный код.Итоговый рабочий код — прошёл функциональный тест (80 000/80 000 доставлено) и TSan без единого предупреждения:

```cpp
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>
#include <memory>
#include <chrono>
#include <new>

// ============================================================
// SPSC ring buffer -- один producer, один consumer на подписчика
// ============================================================
template<typename T, size_t Capacity>
class SPSCQueue {
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be power of 2");
    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> write_pos_{0};
    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> read_pos_{0};
    T buffer_[Capacity];
public:
    bool push(T value) {
        size_t w = write_pos_.load(std::memory_order_relaxed);
        size_t r = read_pos_.load(std::memory_order_acquire);
        if (w - r >= Capacity) return false;
        buffer_[w & (Capacity - 1)] = std::move(value);
        write_pos_.store(w + 1, std::memory_order_release);
        return true;
    }
    bool pop(T& result) {
        size_t r = read_pos_.load(std::memory_order_relaxed);
        size_t w = write_pos_.load(std::memory_order_acquire);
        if (r == w) return false;
        result = std::move(buffer_[r & (Capacity - 1)]);
        read_pos_.store(r + 1, std::memory_order_release);
        return true;
    }
};

struct Subscriber {
    int id;
    SPSCQueue<std::string, 1024> inbox;
    std::atomic<long long> received{0};
};

// ============================================================
// BroadcastChannel: copy-on-write список подписчиков (RCU-style)
// ============================================================
class BroadcastChannel {
public:
    using SubscriberList = std::vector<std::shared_ptr<Subscriber>>;

private:
    std::atomic<std::shared_ptr<const SubscriberList>> subscribers_;

public:
    BroadcastChannel() {
        subscribers_.store(std::make_shared<const SubscriberList>());
    }

    void subscribe(std::shared_ptr<Subscriber> sub) {
        std::shared_ptr<const SubscriberList> old_list, new_list;
        do {
            old_list = subscribers_.load(std::memory_order_acquire);
            auto copy = std::make_shared<SubscriberList>(*old_list);
            copy->push_back(sub);
            new_list = std::move(copy);
        } while (!subscribers_.compare_exchange_weak(
                     old_list, new_list,
                     std::memory_order_acq_rel, std::memory_order_acquire));
    }

    void unsubscribe(int subscriber_id) {
        std::shared_ptr<const SubscriberList> old_list, new_list;
        do {
            old_list = subscribers_.load(std::memory_order_acquire);
            auto copy = std::make_shared<SubscriberList>();
            copy->reserve(old_list->size());
            for (auto& s : *old_list) {
                if (s->id != subscriber_id) copy->push_back(s);
            }
            new_list = std::move(copy);
        } while (!subscribers_.compare_exchange_weak(
                     old_list, new_list,
                     std::memory_order_acq_rel, std::memory_order_acquire));
    }

    // ВАЖНО: если подписчик отписался, но publisher уже держит
    // старый snapshot, он всё ещё видит этот shared_ptr<Subscriber>.
    // Без границы retry зависший/отключённый подписчик заблокировал
    // бы доставку ВСЕМ остальным -- отсюда MAX_RETRY + drop.
    void publish(const std::string& message) {
        std::shared_ptr<const SubscriberList> snapshot =
            subscribers_.load(std::memory_order_acquire);

        constexpr int MAX_RETRY = 1000;
        for (const auto& sub : *snapshot) {
            int attempts = 0;
            while (!sub->inbox.push(message)) {
                if (++attempts >= MAX_RETRY) {
                    std::cerr << "[broadcast] subscriber " << sub->id
                              << " not draining, dropping message\n";
                    break;
                }
                std::this_thread::yield();
            }
        }
    }
};
```

## Ключевая идея — почему COW + per-subscriber SPSC лучше общего broadcast-канала

**Общий MPMC/MPSC broadcast-канал** означал бы: все подписчики читают из **одной** структуры, и там нужна была бы либо фильтрация "это сообщение не моё" на каждого читателя, либо сложная схема с reference counting на каждое сообщение (сколько подписчиков ещё не прочитали — когда все прочитали, можно освобождать). Это создаёт **contention между самими подписчиками** за общую структуру — ровно то, чего лучше избегать.

**COW-список + persubscriber SPSC** разносит это на два независимых механизма:

- `publish()` — просто **один `atomic load`** shared_ptr'а (снимок списка), затем независимая работа с каждым SPSC — **никакого contention между publish-вызовами и между подписчиками друг с другом**.
- `subscribe`/`unsubscribe` — редкие операции, единственные, что платят цену (копирование всего вектора + CAS-retry), что приемлемо, потому что подключение/отключение случается на порядки реже, чем broadcast сообщений.

## RCU-семантика — "eventually consistent" членство

Snapshot, который держит `publish()`, может быть на долю секунды устаревшим относительно только что вызванного `subscribe()`/`unsubscribe()` — это **осознанный компромисс**, а не баг: новый подписчик может не получить одно-два сообщения, отправленных прямо в момент его подключения, а отписавшийся может получить одно лишнее после `unsubscribe()`. Для broadcast-паттерна (presence, статусы, чат-сообщения с идемпотентной доставкой) это обычно приемлемо; там, где нужна строгая консистентность членства, RCU-подход не годится.

## Реальный баг, который я поймал при тестировании (стоит знать заранее)

Первая версия `publish()` с **бесконечным retry** зависала намертво: если подписчик отписывался, пока publisher ещё держал старый snapshot с ссылкой на него, publisher вечно пытался писать в очередь, которую больше никто не читает — **и блокировал доставку всем остальным подписчикам** (publish идёт последовательно по списку). `MAX_RETRY` + drop с логом — обязательная защита, не опциональная оптимизация, для любой системы, где consumer может исчезнуть, пока producer держит на него ссылку.

**Сборка и тесты:**

```bash
g++ -std=c++20 -O2 -pthread broadcast_cow.cpp -o broadcast_cow -latomic
./broadcast_cow

g++ -std=c++20 -O0 -g -fsanitize=thread -pthread broadcast_cow.cpp -o broadcast_cow_tsan -latomic
./broadcast_cow_tsan
```

Готовы перейти к **Backpressure** (следующий пункт Блока 3 — уже частично затронут через `MAX_RETRY`, но стоит формализовать policy: drop oldest vs drop newest vs disconnect), или к **Reclamation problem** (что происходит с памятью подписчика при `unsubscribe`, пока publisher ещё держит на него shared_ptr — мы полагались на refcount, но стоит явно разобрать, чем это отличается от hazard pointers для этого конкретного случая)?


---
---

### **Backpressure**: что делать, если consumer медленнее producer — bounded queue + drop policy vs unbounded + memory blowup

Вижу два момента для исправления: расчёт MB неверный (лишнее деление), и DropOldest не считает реально потерянные сообщения (они дропаются молча внутри `push`, но метод возвращает `true`). Поправлю для честности данных.Теперь метрики честные. За 1.5 секунды unbounded-очередь выросла до **2 ГБ памяти** и продолжила бы расти — реальный OOM-сценарий при медленном клиенте и долгой сессии.

## Разбор результатов

**DisconnectConsumer** — самая показательная policy: `pushed=64` (ровно capacity), затем **немедленно** обнаружена проблема и producer остановлен — вместо бесконечных попыток впихнуть данные не успевающему клиенту, система явно признаёт "этот consumer не справляется" и прекращает работу с ним за один цикл переполнения.

**DropOldest** — `pushed=2 966 657`, но `overwritten=2 966 496` — то есть из почти 3 миллионов "успешных" push реально сохранилось только 64 (то, что осталось в очереди на момент остановки) плюс 97 уже потреблённых. Формально `push()` возвращал `true` почти всегда, но **99.98% сообщений были молча выброшены** сразу после записи — это архитектурно нормально для presence-обновлений ("печатает…", координаты курсора), где важно только последнее состояние, но было бы катастрофой для истории сообщений чата.

**DropNewest** — `dropped=3 534 030` явных потерь, `pushed=160` реально попало в очередь — новые сообщения теряются, пока в очереди остаётся место для более старых, что подходит, если порядок/полнота истории важнее полноты потока.

## Код (ключевые части)

```cpp
enum class DropPolicy { BlockRetry, DropNewest, DropOldest, DisconnectConsumer };

bool push(std::string v) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (data_.size() < Capacity) {
        data_.push_back(std::move(v));
        return true;
    }
    switch (policy_) {
        case DropPolicy::DropNewest:
            return false; // новое теряется, старое остаётся

        case DropPolicy::DropOldest:
            data_.pop_front();
            data_.push_back(std::move(v));
            overwritten_.fetch_add(1, std::memory_order_relaxed);
            return true; // новое принято, старое реально потеряно

        case DropPolicy::DisconnectConsumer:
            consumer_disconnected_.store(true, std::memory_order_release);
            return false; // явный сигнал наверх: клиент не успевает, отключить

        case DropPolicy::BlockRetry:
            return false; // обрабатывается в push_blocking снаружи
    }
    return false;
}

// Для BlockRetry: producer явно ждёт с таймаутом, а не молча теряет данные
bool push_blocking(std::string v, std::chrono::milliseconds timeout) {
    auto deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline) {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            if (data_.size() < Capacity) {
                data_.push_back(std::move(v));
                return true;
            }
        }
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    return false; // не дождались -- вызывающий код сам решает: разорвать соединение, залогировать, и т.д.
}
```

## Как выбирать policy для реального чата

|Тип данных|Рекомендуемая policy|Почему|
|---|---|---|
|Сообщения чата (текст, файлы)|**DisconnectConsumer** или `BlockRetry` с коротким таймаутом|Потеря сообщения — баг с точки зрения пользователя; лучше явно разорвать соединение и заставить клиента переподключиться с полной синхронизацией, чем тихо терять часть истории|
|Presence / "печатает…" / online-статус|**DropOldest**|Важно только последнее состояние; старое устарело в момент, когда пришло новое|
|Метрики / телеметрия|**DropNewest**|Не критично потерять недавние точки при всплеске нагрузки, важнее не тратить ресурсы на бесконечный retry|
|Критичные системные события (модерация, бан)|`BlockRetry` с достаточным таймаутом, либо отдельный **приоритетный** канал с большей capacity|Эти события не должны теряться вообще — иногда стоит завести для них отдельную структуру с более щедрым бюджетом памяти|

## Связь с MAX_RETRY из broadcast-демо

`MAX_RETRY` + drop в предыдущем `BroadcastChannel::publish()` — это фактически гибрид `BlockRetry` (ограниченный по числу попыток, не по времени) и `DropNewest` (после исчерпания попыток теряем именно новое сообщение для конкретного зависшего подписчика, а не всю рассылку). В реальном проде эту policy стоит параметризовать так же, как здесь — разным типам broadcast-сообщений (обычный текст vs presence) нужна разная стратегия.

**Полный код** (`backpressure_demo.cpp`) компилируется и запускается так же, как предыдущие демки:

```bash
g++ -std=c++20 -O2 -pthread backpressure_demo.cpp -o backpressure_demo
./backpressure_demo
```

Готовы перейти к последнему пункту Блока 3 — **reclamation problem** для broadcast-подписчиков (что происходит с памятью при disconnect, пока publisher ещё держит snapshot), или собрать всё в единый прототип чат-шины, как изначально планировали?

---
---

### **Reclamation проблема**: если пользователь отключается, когда безопасно освободить его структуры при lock-free доступе (hazard pointers / epoch-based / RCU)

Это ровно та reclamation-проблема, которую мы уже неявно решили в `broadcast_cow.cpp` через `shared_ptr` — но стоит явно показать **почему** это работает и **какой ценой**, а не просто "работает и ладно".Именно то, что нужно было доказать: `unsubscribe(1)` выполнился **между** взятием snapshot'а и его использованием, но "УДАЛЁН" для Subscriber 1 появляется **только после** строки "отправляю... подписчику 1" — то есть публикация безопасно отработала с уже "удалённым" (с точки зрения канала) подписчиком, и реальное освобождение памяти произошло ровно в момент выхода `snapshot` из scope, не раньше.

## Разбор — почему это работает и это НЕ hazard pointers, НЕ epoch-based

Каждый `shared_ptr<Subscriber>` внутри `SubscriberList` — это независимый refcount. Когда `publish()` берёт `snapshot = subscribers_.load()`, он получает `shared_ptr<const SubscriberList>` — refcount инкрементится **один раз** для всего вектора, а не по разу на каждого подписчика (вектор просто хранит копии `shared_ptr<Subscriber>`, которые были скопированы туда во время `subscribe()`/`unsubscribe()`, а не во время `publish()`).

Когда `unsubscribe(1)` строит новый вектор, он **не трогает** старый — старый вектор (тот, что держит `snapshot`) продолжает жить, пока у него есть хоть одна ссылка. `sub1`'s refcount остаётся ≥ 1, потому что старый вектор (внутри snapshot publisher'а) всё ещё хранит на него `shared_ptr`. Только когда `snapshot` разрушается на выходе из `publish_slow()` — refcount вектора падает до нуля → вектор разрушается → refcount каждого `shared_ptr<Subscriber>` внутри падает → если это была последняя ссылка на конкретного Subscriber, **тогда** вызывается его деструктор.

## Сравнение с hazard pointers и epoch-based — в чём принципиальная разница

||shared_ptr refcounting (наш случай)|Hazard Pointers|Epoch-based (RCU)|
|---|---|---|---|
|**Что отслеживается**|Число активных ссылок на **конкретный объект**|Явный список "кто сейчас держит какой указатель"|Глобальный счётчик "поколений", в какой эпохе кто находится|
|**Когда освобождается**|Точно в момент, когда счётчик достиг нуля — сразу, без задержки|Сразу, как только объект больше ни в одном hazard-слоте|Пакетно — когда все потоки "продвинулись" за эпоху удаления|
|**Стоимость на чтение**|Atomic inc/dec refcount при каждом **копировании** shared_ptr (здесь — не на каждый publish, а только при создании нового snapshot внутри subscribe/unsubscribe)|Atomic store hazard-слота + периодическая проверка scan|Atomic load эпохи при входе в критическую секцию|
|**Нужен ли явный "protected" список**|Нет — сам механизм refcounting уже это даёт|Да, отдельная инфраструктура|Да, отдельная инфраструктура|

**Ключевое отличие: `shared_ptr` даёт reclamation "из коробки" за счёт того, что каждый объект уже несёт свой счётчик** — не нужно городить отдельный per-thread hazard-реестр или эпохи, как в Treiber stack на raw pointers из более ранних разборов. Цена — тот же overhead, что разбирали для `atomic<shared_ptr<T>>`: atomic refcount operations при каждом копировании `shared_ptr`, и потенциальная нелок-фри реализация внутри самого `atomic<shared_ptr<...>>` на некоторых компиляторах (стоит проверить `is_lock_free()`, как обсуждали).

## Почему для broadcast-канала это оправданный выбор, а не для Treiber stack

**Частота операций разная.** В Treiber stack push/pop — это _hot path_, тысячи операций в секунду на структуру с миллионами узлов, там refcount-overhead на _каждую_ операцию заметен. В broadcast-канале refcount-инкремент происходит только при `subscribe`/`unsubscribe` (создание нового вектора с копиями shared_ptr) — **редко**, а сам `publish()` лишь читает уже существующие `shared_ptr` по ссылке (`const auto& sub`), не копируя и не трогая refcount вообще. Именно поэтому в комментариях к коду это было отмечено как "оправданное использование" — переиспользование дорогого механизма именно там, где он почти ничего не стоит.

## Практический вывод для собеседования

Если спросят "как вы решаете reclamation problem для списка подписчиков при disconnect" — правильный ответ не "используем hazard pointers всегда", а: **выбор механизма reclamation зависит от частоты read/write операций конкретной структуры.** Для hot-path lock-free структур (очереди, стеки с миллионами операций/сек) — hazard pointers или epoch-based, потому что там refcount-overhead был бы недопустим. Для структур с редкими мутациями и частыми, но дешёвыми чтениями (списки подписчиков, конфигурация, roster) — `atomic<shared_ptr<T>>` через COW часто оптимальнее по соотношению "простота кода / производительность", и вы уже получили reclamation бесплатно, без отдельной инфраструктуры.

Это закрывает последний пункт вашего плана Блока 3. Мы прошли весь путь от memory_order до полного прототипа чат-сервера (epoll-реактор → sharded воркеры → per-client SPSC → COW broadcast → backpressure → reclamation). Хотите собрать финальную сводку по всем трём блокам в виде документа/шпаргалки для подготовки к собеседованию, или есть конкретные темы, которые стоит повторить отдельно?

---
---

### **Практика:** собрать минимальный прототип — N потоков-писателей кладут сообщения в шину, M потоков-читателей (эмулируют коннекты) их разбирают, с метриками latency/throughput под нагрузкой.

Финальная практика — соберу самостоятельный прототип: N писателей кладут в общую bounded MPMC-шину, M читателей её разбирают, с полными метриками latency (p50/p99/p999) и throughput под нагрузкой.Все три сценария прошли без потерь (`dropped=0` везде), 8-9M msg/sec throughput, latency в диапазоне сотен микросекунд. Проверю на TSan для полной уверенности:TSan чист — все три сценария прошли без единого предупреждения о гонках. Вот полный код:

```cpp
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <algorithm>
#include <new>
#include <cassert>
#include <iomanip>

// ============================================================
// MPMC bus (Vyukov) -- итоговая версия из всех разборов:
// padding против false sharing, bounded с явным backpressure.
// ============================================================
template<typename T>
class MPMCBus {
    struct Cell { std::atomic<size_t> sequence; T data; };

    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> enqueue_pos_;
    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> dequeue_pos_;
    Cell* buffer_;
    size_t buffer_mask_;

public:
    explicit MPMCBus(size_t capacity)
        : buffer_(new Cell[capacity]), buffer_mask_(capacity - 1) {
        assert((capacity >= 2) && ((capacity & (capacity - 1)) == 0));
        for (size_t i = 0; i < capacity; ++i)
            buffer_[i].sequence.store(i, std::memory_order_relaxed);
        enqueue_pos_.store(0, std::memory_order_relaxed);
        dequeue_pos_.store(0, std::memory_order_relaxed);
    }
    ~MPMCBus() { delete[] buffer_; }
    MPMCBus(const MPMCBus&) = delete;

    bool try_push(T value) {
        Cell* cell; size_t pos = enqueue_pos_.load(std::memory_order_relaxed);
        for (;;) {
            cell = &buffer_[pos & buffer_mask_];
            size_t seq = cell->sequence.load(std::memory_order_acquire);
            intptr_t dif = (intptr_t)seq - (intptr_t)pos;
            if (dif == 0) {
                if (enqueue_pos_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) break;
            } else if (dif < 0) return false; // шина полна -- backpressure наверх
            else pos = enqueue_pos_.load(std::memory_order_relaxed);
        }
        cell->data = std::move(value);
        cell->sequence.store(pos + 1, std::memory_order_release);
        return true;
    }

    bool try_pop(T& result) {
        Cell* cell; size_t pos = dequeue_pos_.load(std::memory_order_relaxed);
        for (;;) {
            cell = &buffer_[pos & buffer_mask_];
            size_t seq = cell->sequence.load(std::memory_order_acquire);
            intptr_t dif = (intptr_t)seq - (intptr_t)(pos + 1);
            if (dif == 0) {
                if (dequeue_pos_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) break;
            } else if (dif < 0) return false; // шина пуста
            else pos = dequeue_pos_.load(std::memory_order_relaxed);
        }
        result = std::move(cell->data);
        cell->sequence.store(pos + buffer_mask_ + 1, std::memory_order_release);
        return true;
    }
};

// Сообщение несёт временную метку отправки -- нужна для честного
// измерения latency: время от push() ДО реального pop() (end-to-end).
struct Message {
    int producer_id;
    long long seq;
    std::chrono::steady_clock::time_point sent_at;
};

struct Metrics {
    std::vector<long long> latencies_ns;
    long long processed = 0;
};

void print_percentiles(const char* label, std::vector<long long>& lat) {
    if (lat.empty()) { std::cout << label << ": нет данных\n"; return; }
    std::sort(lat.begin(), lat.end());
    auto pct = [&](double p) {
        size_t idx = static_cast<size_t>(p * (lat.size() - 1));
        return lat[idx];
    };
    std::cout << label << " (n=" << lat.size() << "):\n"
              << "  p50:  " << std::setw(8) << pct(0.50) << " ns\n"
              << "  p90:  " << std::setw(8) << pct(0.90) << " ns\n"
              << "  p99:  " << std::setw(8) << pct(0.99) << " ns\n"
              << "  p999: " << std::setw(8) << pct(0.999) << " ns\n"
              << "  max:  " << std::setw(8) << lat.back() << " ns\n";
}

struct PrototypeResult {
    long long total_produced = 0;
    long long total_consumed = 0;
    long long total_dropped = 0;
    double wall_time_sec = 0;
    std::vector<long long> all_latencies_ns;
};

PrototypeResult run_prototype(int num_writers, int num_readers,
                                int messages_per_writer, size_t bus_capacity) {
    MPMCBus<Message> bus(bus_capacity);
    const long long total_target = static_cast<long long>(num_writers) * messages_per_writer;

    std::atomic<long long> produced{0}, consumed{0}, dropped{0};

    std::vector<Metrics> reader_metrics(num_readers);
    for (auto& m : reader_metrics) m.latencies_ns.reserve(messages_per_writer * num_writers / num_readers + 16);

    auto start = std::chrono::steady_clock::now();

    std::vector<std::thread> writers;
    for (int w = 0; w < num_writers; ++w) {
        writers.emplace_back([&, w] {
            for (int i = 0; i < messages_per_writer; ++i) {
                Message msg{w, i, std::chrono::steady_clock::now()};
                int attempts = 0;
                constexpr int MAX_ATTEMPTS = 10000;
                while (!bus.try_push(msg)) {
                    if (++attempts >= MAX_ATTEMPTS) {
                        dropped.fetch_add(1, std::memory_order_relaxed);
                        goto next_message;
                    }
                    std::this_thread::yield();
                }
                produced.fetch_add(1, std::memory_order_relaxed);
                next_message:;
            }
        });
    }

    std::vector<std::thread> readers;
    for (int r = 0; r < num_readers; ++r) {
        readers.emplace_back([&, r] {
            Message msg;
            auto& metrics = reader_metrics[r];
            while (consumed.load(std::memory_order_relaxed) + dropped.load(std::memory_order_relaxed) < total_target) {
                if (bus.try_pop(msg)) {
                    auto now = std::chrono::steady_clock::now();
                    auto latency_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now - msg.sent_at).count();
                    metrics.latencies_ns.push_back(latency_ns);
                    metrics.processed++;
                    consumed.fetch_add(1, std::memory_order_relaxed);
                } else {
                    std::this_thread::yield();
                }
            }
        });
    }

    for (auto& t : writers) t.join();
    for (auto& t : readers) t.join();

    auto end = std::chrono::steady_clock::now();

    PrototypeResult result;
    result.total_produced = produced.load();
    result.total_consumed = consumed.load();
    result.total_dropped = dropped.load();
    result.wall_time_sec = std::chrono::duration<double>(end - start).count();
    for (auto& m : reader_metrics) {
        result.all_latencies_ns.insert(result.all_latencies_ns.end(),
                                        m.latencies_ns.begin(), m.latencies_ns.end());
    }
    return result;
}

void print_result(const char* label, const PrototypeResult& r) {
    std::cout << "\n===== " << label << " =====\n";
    std::cout << "produced=" << r.total_produced << ", consumed=" << r.total_consumed
              << ", dropped=" << r.total_dropped << "\n";
    std::cout << "wall time: " << r.wall_time_sec << " s\n";
    std::cout << "throughput: " << static_cast<long long>(r.total_consumed / r.wall_time_sec) << " msg/sec\n";
    auto lat_copy = r.all_latencies_ns;
    print_percentiles("end-to-end latency", lat_copy);
}

int main() {
    constexpr size_t BUS_CAPACITY = 4096;
    constexpr int MESSAGES_PER_WRITER = 100'000;

    auto r1 = run_prototype(4, 4, MESSAGES_PER_WRITER, BUS_CAPACITY);
    print_result("4 writers / 4 readers", r1);

    auto r2 = run_prototype(8, 2, MESSAGES_PER_WRITER / 2, BUS_CAPACITY);
    print_result("8 writers / 2 readers (reader-bound)", r2);

    auto r3 = run_prototype(2, 8, MESSAGES_PER_WRITER, BUS_CAPACITY);
    print_result("2 writers / 8 readers (writer-bound)", r3);

    return 0;
}
```

**Сборка и запуск:**

```bash
g++ -std=c++20 -O2 -pthread final_prototype.cpp -o final_prototype
./final_prototype

# Обязательная проверка на гонки (уменьшите MESSAGES_PER_WRITER для скорости под TSan):
g++ -std=c++20 -O0 -g -fsanitize=thread -pthread final_prototype.cpp -o final_prototype_tsan
./final_prototype_tsan
```

## Что показывают три сценария

**4/4 симметричный** — baseline: 8.1M msg/sec, p50 245 мкс. p999 (833 мкс) заметно выше p50 — типичный длинный хвост из-за yield-based busy-wait вместо более умного backoff.

**8 writers / 2 readers (reader-bound)** — читатели становятся узким местом (2 потока разбирают вдвое больший поток писателей), но throughput **не упал**, а даже чуть вырос (9.3M msg/sec) — потому что per-slot contention MPMC-очереди хорошо масштабируется на запись даже при малом числе читателей. p99 здесь лучше (350 мкс vs 683 мкс) — меньше читателей означает меньше конкуренции за `dequeue_pos_`.

**2 writers / 8 readers (writer-bound)** — читатели простаивают (yield-loop), пока писателей мало — самая низкая latency на всех перцентилях (p999 всего 356 мкс) и самый чистый хвост, потому что как только сообщение появляется, кто-то из 8 читателей почти сразу его подхватывает.

## Важная методологическая оговорка

Этот контейнер даёт **1 видимый CPU** — реальной параллельности между потоками физически нет, только чередование планировщиком. Абсолютные цифры throughput здесь **не показательны** для настоящей многоядерной машины (там ожидается на порядок выше throughput при реальной параллельности, но и заметно более выраженная разница между reader-bound/writer-bound сценариями из-за реальной cache-line contention между ядрами, а не только на одном ядре). Обязательно перегоните это на своей рабочей машине — особенно интересно посмотреть на `perf c2c` при 8/8 конфигурации, чтобы увидеть реальный false-sharing/contention паттерн на `enqueue_pos_`/`dequeue_pos_`.

Этим прототипом закрывается вся практика Блока 3, а вместе с ней и весь план — от memory model до полной архитектуры чат-сервера с метриками под нагрузкой.

---
---
