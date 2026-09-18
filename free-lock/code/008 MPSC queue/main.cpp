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
