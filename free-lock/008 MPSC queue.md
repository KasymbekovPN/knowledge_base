
###  **MPSC queue** (Michael-Scott или на основе intrusive linked list) — уже ближе к реальному broadcast-паттерну

```cpp
#include <atomic>
#include <iostream>
#include <format>
#include <thread>
#include <chrono>
#include <new>

namespace {
    // ============================================================
    // Production-вариант intrusive MPSC queue (алгоритм Vyukov)
    // + node pool (freelist) вместо new/delete на каждое сообщение
    // + padding против false sharing между head_ и tail_
    // ============================================================

    template<typename T>
    class MPSCQueue {
    public:
        struct Node {
            std::atomic<Node*> next{nullptr};
            T value;
            explicit Node(T value) : value{std::move(value)} {}
            Node(): value(T{}) {}
        };

    private:
        // head_ пишут ВСЕ producer'ы -- своя кэш-линия
        alignas(std::hardware_destructive_interference_size)
            std::atomic<Node*> head_;
        // tail_ трогает ТОЛЬКО consumer -- своя кэш-линия,
        // иначе каждый enqueue() бил бы по кэш-линии consumer'а
        // и наоборот (ровно false sharing из блока 1)
        alignas(std::hardware_destructive_interference_size)
            Node* tail_;

        Node stub_;

    public:
        MPSCQueue() {
            head_.store(&stub_, std::memory_order_relaxed);
            tail_ = &stub_;
        }

        void enqueue(Node* node) {
            node->next.store(nullptr, std::memory_order_relaxed);
            Node* prev{head_.exchange(node, std::memory_order_acq_rel)};
            prev->next.store(node, std::memory_order_release);
        }

        Node* dequeue() {
            Node* t{tail_};
            Node* next{t->next.load(std::memory_order_acquire)};

            if (t == &stub_) {
                if (next == nullptr) return nullptr;
                tail_ = next;
                t = next;
                next = next->next.load(std::memory_order_acquire);
            }

            if (next != nullptr) {
                tail_ = next;
                return t;
            }

            Node* h{head_.load(std::memory_order_acquire)};
            if (t != h) return nullptr; // producer в процессе публикации

            enqueue(&stub_);
            next = t->next.load(std::memory_order_acquire);
            if (next != nullptr) {
                tail_ = next;
                return t;
            }
            return nullptr;
        }

        [[nodiscard]] bool empty() const {
            return
                tail_->next.load(std::memory_order_acquire) == nullptr &&
                    head_.load(std::memory_order_acquire) == tail_;
        }

    };

    // ============================================================
    // Node pool: lock-free freelist (по сути Treiber stack из
    // предыдущих разборов), переиспользует узлы вместо new/delete.
    // Свободные узлы хранятся через тот же intrusive next-указатель.
    // ============================================================
    template<typename T>
    class NodePool {
    public:
        using Node = MPSCQueue<T>::Node;

    private:
        alignas(std::hardware_destructive_interference_size)
            std::atomic<Node*> free_list_{nullptr};

    public:
        ~NodePool() {
            Node* n{free_list_.load(std::memory_order_relaxed)};
            while (n) {
                Node* next = n->next.load(std::memory_order_relaxed);
                delete n;
                n = next;
            }
        }

        Node* acquire(T value) {
            Node* old_head{free_list_.load(std::memory_order_acquire)};
            while (old_head) {
                if (Node* next{old_head->next.load(std::memory_order_relaxed)};
                    free_list_.compare_exchange_weak(
                    old_head,
                    next,
                    std::memory_order_acq_rel,
                    std::memory_order_acquire)) {

                    old_head->value = std::move(value);
                    return old_head;
                }
            }

            // freelist пуст -- реальная аллокация
            return new Node{std::move(value)};
        }

        void release(Node* n) {
            Node* old_head{free_list_.load(std::memory_order_relaxed)};
            do {
                n->next.store(old_head, std::memory_order_relaxed);
            } while (!free_list_.compare_exchange_weak(
                old_head,
                n,
                std::memory_order_acq_rel,
                std::memory_order_relaxed));
        }

    };

}

int main(int argc, char *argv[]) {
    using item_type = int;
    using Queue = MPSCQueue<item_type>;
    Queue queue;
    NodePool<item_type> pool;

    constexpr int NUM_PRODUCERS{4};
    constexpr int ITEMS_PER_PRODUCER{250'000};
    constexpr int TOTAL{NUM_PRODUCERS * ITEMS_PER_PRODUCER};

    std::atomic<long long> checksum_in{0};
    std::atomic<long long> checksum_out{0};
    std::atomic<int> consumed{0};

    const auto start{std::chrono::steady_clock::now()};
    std::vector<std::thread> producers;
    for (int p{}; p < NUM_PRODUCERS; ++p) {
        producers.emplace_back([&pool, &checksum_in, &queue, p] {
            for (int i{}; i < ITEMS_PER_PRODUCER; ++i) {
                const int value{p * ITEMS_PER_PRODUCER + i};
                Queue::Node* node{pool.acquire(value)};
                queue.enqueue(node);
                checksum_in.fetch_add(value, std::memory_order_relaxed);
            }
        });
    }

    std::thread consumer{[&consumed, &queue, &checksum_out, &pool] {
        while (consumed.load(std::memory_order_relaxed) < TOTAL) {
            if (Queue::Node* node{queue.dequeue()}) {
                checksum_out.fetch_add(node->value, std::memory_order_relaxed);;
                consumed.fetch_add(1, std::memory_order_relaxed);
                pool.release(node);
            } else {
                std::this_thread::yield();
            }
        }
    }};

    for (auto& t: producers) t.join();
    consumer.join();

    const auto end{std::chrono::steady_clock::now()};
    const auto ms{std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()};

    std::cout << std::format("Total processed: {} / {}\n", consumed.load(), TOTAL) << std::flush;
    const auto cs_in{checksum_in.load()};
    const auto cs_out{checksum_out.load()};
    std::cout << std::format("Checksum: in {} / out {} => {}\n",
        cs_in,
        cs_out,
        cs_in == cs_out ? "success" : "fail") << std::flush;
    std::cout << std::format("Duration: {} ms\n", ms) << std::flush;

    return 0;
}

```

## Ключевая идея алгоритма Vyukov

**`head_`** — точка, куда пишут producer'ы (атомарный `exchange`, единственная точка конкуренции между ними). **`tail_`** — точка, откуда читает единственный consumer, и она **вообще не atomic** — потому что читатель один, ему не с кем конкурировать за неё. Это и даёт основной выигрыш производительности по сравнению с полным Michael-Scott (там и head, и tail — atomic с CAS-retry loop на обеих сторонах, потому что там multi-consumer).

## Разбор enqueue — почему нет CAS-цикла вообще

```cpp
void enqueue(Node* n) {
    n->next.store(nullptr, std::memory_order_relaxed);
    Node* prev = head_.exchange(n, std::memory_order_acq_rel);
    prev->next.store(n, std::memory_order_release);
}
```

Обычно lock-free enqueue — это `while(!compare_exchange...)`. Здесь вместо CAS-цикла используется **`exchange`** — безусловная атомарная замена, которая **никогда не проваливается** (в отличие от CAS, она не сравнивает "старое" значение — просто меняет и возвращает, что было). Это лучше по производительности: нет retry-петли, нет contention-деградации при росте числа producer'ов, каждый вызов `exchange` гарантированно завершается за одну атомарную операцию.

**Цена этого решения** — временное "окно неконсистентности": между `exchange` (узел уже логически в очереди с точки зрения producer'а) и `prev->next.store()` (связь ещё не опубликована) проходит какое-то время. Если consumer в этот момент попытается пройти по цепочке через `prev`, он увидит `prev->next == nullptr`, хотя реально узел уже "добавлен". Именно это и обрабатывает возврат `nullptr` из `dequeue()` при `t != h` — это не "очередь пуста", а "подожди, producer ещё не закончил публикацию".

## Разбор dequeue — зачем нужен stub

Стаб (`stub_`) решает проблему "что делать, когда снимаем последний реальный элемент — на что указывать `tail_` дальше". Без стаба пришлось бы отдельно обрабатывать переход "queue has 1 element → queue is empty" как особый случай. Со стабом: когда снимаем последний узел, `enqueue(&stub_)` кладёт стаб обратно в очередь — теперь всегда есть "якорь", от которого можно оттолкнуться на следующем dequeue, даже если очередь физически пуста.

## Применение к архитектуре чата

Это прямая реализация паттерна из Блока 3: N воркеров чат-логики (producers) кладут исходящие сообщения в очередь одного connection-writer'а (consumer) на конкретного клиента. У каждого подключённого пользователя — своя `MPSCQueue<Message>`, воркеры пишут в неё через `enqueue()` без блокировок, а один network-поток на этот коннект вычитывает через `dequeue()` и отправляет в сокет.

**Практические доработки для продакшена:**

- **Node pool** вместо `new`/`delete` на каждое сообщение — аллокации на hot path чата дороги, стоит переиспользовать узлы через lock-free pool или thread-local freelist.
- **Backpressure**: если consumer не успевает вычитывать (клиент медленный/отвалился), очередь растёт неограниченно — нужен bounded вариант с ограничением размера и policy на переполнение (drop oldest / drop newest / disconnect client).
- **Padding** между `head_` и `tail_` — они false-share, если лежат в одной кэш-линии (ровно тот паттерн, что разбирали в блоке про false sharing) — стоит явно развести через `alignas(std::hardware_destructive_interference_size)`.

## Что важно про этот pool

`NodePool` — это, по сути, Treiber stack из самых первых разборов, переиспользованный как freelist: `acquire()` — Treiber pop, `release()` — Treiber push, с тем же `compare_exchange_weak` в цикле, что уже разбирали. ABA здесь теоретически возможна (тот же freelist-адрес мог бы вернуться на другой узел между `load` и `CAS`), но на практике для freelist это не проблема, потому что узел либо в очереди, либо в freelist, никогда не в обоих сразу, и указатель либо валиден, либо это тот же самый физический слот с корректным содержимым — в отличие от Treiber stack с произвольными данными, тут ABA не приводит к логической ошибке, только к возможному "лишнему" retry.

**Что ещё стоило бы добавить для реального продакшена:** bounded-версия (лимит на размер очереди + policy при переполнении) и метрика latency (время от `enqueue` до `dequeue`) — оба пункта прямо из архитектурного блока 3 про backpressure. 
