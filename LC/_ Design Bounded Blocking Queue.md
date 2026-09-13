
## Суть задачи

Реализовать очередь с фиксированной ёмкостью (capacity), поддерживающую конкурентный доступ:

- `enqueue(element)` — добавить элемент; если очередь **полна**, вызывающий поток должен **заблокироваться** и ждать, пока не появится место
- `dequeue()` — забрать элемент; если очередь **пуста**, вызывающий поток должен **заблокироваться** и ждать, пока не появится элемент
- `size()` — текущее количество элементов

Это классический **producer-consumer** с ограниченным буфером — пожалуй, самый практически важный паттерн из всего concurrency-раздела, потому что почти любой реальный пайплайн обработки данных так устроен.

## Идея

В отличие от LC1114/1115/1117, где синхронизация была вокруг **одного события или маленького счётчика**, здесь нужна структура данных (сама очередь) **плюс** два независимых условия ожидания:

- producer ждёт "not full"
- consumer ждёт "not empty"

Один `mutex` защищает саму очередь (иначе push/pop из разных потоков — гонка на внутреннем состоянии `std::queue`), а `condition_variable` — **две штуки**, по одной на каждое условие (можно обойтись и одной cv с двумя предикатами, но раздельные cv эффективнее — не будят producer'ов, когда сигнал предназначен consumer'ам, и наоборот).

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <array>
#include <atomic>
#include <format>

namespace {

    template <typename T>
    class BoundedBlockingQueue {
        std::queue<T> buffer;
        size_t capacity;
        std::mutex mtx;
        std::condition_variable not_full;
        std::condition_variable not_empty;

    public:
        explicit BoundedBlockingQueue(const size_t capacity) : capacity{capacity} {}

        void enqueue(T item) {
            std::unique_lock<std::mutex> lock{mtx};
            not_full.wait(lock, [this] { return buffer.size() < capacity; });
            buffer.push(std::move(item));
            not_empty.notify_one();
        }

        T dequeue() {
            std::unique_lock<std::mutex> lock{mtx};
            not_empty.wait(lock, [this] { return !buffer.empty(); });
            T item = std::move(buffer.front());
            buffer.pop();
            not_full.notify_one();

            return item;
        }

        size_t size() {
            std::lock_guard<std::mutex> lock{mtx};
            return buffer.size();
        }
    };

    template <typename T, size_t CAPACITY>
    class SPSCRingBuffer {
        std::array<T, CAPACITY> buffer;
        std::atomic<size_t> head{0}; // индекс записи, трогает только producer
        std::atomic<size_t> tail{0}; // индекс чтения, трогает только consumer

        public:
            bool try_enqueue(const T& item) {
                const size_t current_head{head.load(std::memory_order_relaxed)};
                const size_t next_head{(current_head + 1) % CAPACITY};
                if (next_head == tail.load(std::memory_order_acquire)) {
                    return false;
                }

                buffer[current_head] = item;
                head.store(next_head, std::memory_order_release);

                return true;
            }

            bool try_dequeue(T& item) {
                const size_t current_tail{tail.load(std::memory_order_relaxed)};
                if (current_tail == head.load(std::memory_order_acquire)) {
                    return false;
                }

                item = buffer[current_tail];
                tail.store((current_tail + 1) % CAPACITY, std::memory_order_release);

                return true;
            }
    };

    void start_test0() {
        BoundedBlockingQueue<int> queue{3};
        std::mutex cout_mtx;

        const auto producer = [&](const int id, const int count) {
            for (int i{}; i < count; ++i) {
                const int value{id * 100 + i};
                queue.enqueue(value);
                {
                    std::lock_guard<std::mutex> lock{cout_mtx};
                    std::cout << std::format("Producer {} -> enqueue {} (size = {})\n", id, value, queue.size());
                }
            }
        };

        const auto consumer = [&](const int id, const int count) {
            for (int i{}; i < count; ++i) {
                const auto value = queue.dequeue();
                {
                    std::lock_guard<std::mutex> lock{cout_mtx};
                    std::cout << std::format("Consumer {} -> dequeue {} (size = {})\n", id, value, queue.size());
                }
            }
        };

        std::thread p1(producer, 1, 5);
        std::thread p2(producer, 2, 5);
        std::thread c1(consumer, 1, 5);
        std::thread c2(consumer, 2, 5);

        p1.join();
        p2.join();
        c1.join();
        c2.join();
    }

    void start_test1() {
        SPSCRingBuffer<int, 3> ring_buffer;
        std::mutex cout_mtx;

        const auto producer = [&](const int id, const int count) {
            for (int i{}; i < count; ++i) {
                if (const int value{id * 100 + i}; ring_buffer.try_enqueue(value))
                {
                    std::lock_guard<std::mutex> lock{cout_mtx};
                    std::cout << std::format("Producer {} -> enqueue {}\n", id, value);
                }
            }
        };

        const auto consumer = [&](const int id, const int count) {
            for (int i{}; i < count; ++i) {
                if (int value; ring_buffer.try_dequeue(value))
                {
                    std::lock_guard<std::mutex> lock{cout_mtx};
                    std::cout << std::format("Consumer {} -> dequeue {}\n", id, value);
                }
            }
        };

        std::thread p1(producer, 1, 5);
        std::thread p2(producer, 2, 5);
        std::thread c1(consumer, 1, 5);
        std::thread c2(consumer, 2, 5);

        p1.join();
        p2.join();
        c1.join();
        c2.join();
    }
}

int main(int argc, char *argv[]) {
    start_test0();
    // start_test1();

    return 0;
}

```

**Почему здесь `notify_one()`, а не `notify_all()`, как в предыдущих задачах** — это важный контраст, который стоит явно проговорить на собеседовании. В LC1114/1115/1117 `notify_all()` был оправдан, потому что могли быть **разные** предикаты у разных ждущих, и нужно было каждому дать шанс перепроверить своё условие. Здесь же ситуация симметричная и количественная: если освободилось **одно** место в очереди — имеет смысл разбудить **ровно одного** ждущего producer'а (не всех сразу — остальные всё равно снова заснут, увидев, что места опять нет, а это лишние переключения контекста и потенциальный "thundering herd"). Использование `notify_one` вместо `notify_all` здесь — не микрооптимизация, а осознанное согласование семантики сигнала с количеством освободившихся ресурсов.

Важно: `size()` в выводе может быть **неточным относительно момента печати** — между вызовом `queue.size()` и выводом в `cout` другой поток может успеть изменить состояние. Это не баг логики очереди, а типичная особенность любого лога из конкурентной программы — значение было корректным **в момент вызова**, но не гарантированно актуально к моменту, когда вы его увидели на экране.

## Что важно проговорить на собеседовании

**1. Почему нельзя обойтись одной condition_variable вместо двух.** Формально можно — `cv.wait(lock, predicate)` всё равно перепроверяет именно свой предикат, так что один `cv` с `notify_all()` тоже дал бы корректный результат: все просыпаются, каждый проверяет своё условие, кому не подходит — снова засыпает. Но это менее эффективно при большом числе потоков: если у вас 100 producer'ов и 100 consumer'ов, каждое изменение будит **всех** 200, а не только тех, кому реально может повезти. Раздельные `notFull`/`notEmpty` с `notify_one()` — целенаправленная побудка, что критично при высокой конкурентности (снова тема "миллионы RPS" из вакансии).

**2. Lock-free альтернатива — ожидаемый follow-up для этой роли.** Классическая блокирующая очередь (как выше) — это то, что вы явно назвали своим более привычным подходом. Продвинутый ответ — **lock-free ring buffer (circular buffer)** для одного producer / одного consumer (SPSC — single-producer-single-consumer), где вместо mutex используются только atomic-индексы head/tail:

Здесь стоит явно проговорить, **почему это вообще безопасно без mutex**: `head` пишет **только** producer, читает и producer, и consumer; `tail` пишет **только** consumer, читает и тот, и другой. Каждая атомарная переменная имеет **единственного писателя** — это ключевое структурное свойство, которое устраняет гонки без блокировок. `memory_order_release` на записи `head`/`tail` гарантирует, что запись в `buffer[currentHead]` (данные) видна другому потоку **до того**, как он увидит обновлённый индекс через `memory_order_acquire` — без этого consumer теоретически мог бы прочитать индекс раньше, чем реальные данные "долетят" до его кэша (переупорядочивание). Это ровно тот "acquire/release" паттерн, о котором уже шла речь в разборе LC1114.

**Ограничение этого подхода** — он работает только для **одного** producer и **одного** consumer (SPSC). Для MPMC (multi-producer-multi-consumer) lock-free-очередь — на порядок сложнее (нужен, например, алгоритм Майкла-Скотта или более продвинутые структуры вроде disruptor pattern), и это уже явно выходит за рамки "уверенного знания", скорее уровень "знаю, что существует, и в общих чертах, как устроено" — если спросят, честный ответ "полная lock-free MPMC-очередь сложна в деталях, но принцип — атомарные CAS-операции на указателях head/tail с обработкой ABA-проблемы" уже покажет достаточную осведомлённость, не переоценивая себя.

**3. Прямая связь с вакансией.** Bounded blocking queue — это ровно то, через что стадии графа исполнения (упомянутого в описании вакансии) обмениваются данными в конвейерной обработке: стадия A производит батчи документов, стадия B их потребляет, ограниченная ёмкость очереди даёт **backpressure** — если стадия B не успевает, стадия A автоматически притормаживается (блокируется на `enqueue`), вместо неограниченного роста памяти. Это прямая альтернатива безлимитной очереди, которая на "миллионах RPS" неизбежно приведёт к OOM при любом временном рассинхроне скоростей стадий.
