#include <atomic>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

namespace {

    template <typename T>
    class TreiberStack {
        struct Node {
            T data;
            Node* next;
            explicit Node(T v): data{std::move(v)}, next{nullptr} {}
        };

        std::atomic<Node*> head{nullptr};

    public:
        void push(T value) {
            Node* new_value{new Node{std::move(value)}};

            // (1) relaxed: пока узел никому не виден, порядок не важен
            new_value->next = head.load(std::memory_order_relaxed);

            // (2) CAS: acq_rel
            //   - acquire-часть: если CAS не удался, нам нужно увидеть
            //     актуальный head для следующей попытки
            //   - release-часть: если CAS удался, публикуем new_node —
            //     всё, что записано в него (data, next) в шаге (1),
            //     станет видно потоку, который потом сделает pop()
            while (!head.compare_exchange_weak(
                new_value->next,
                new_value,
                std::memory_order_acq_rel,
                std::memory_order_relaxed)) {
                // new_node->next уже обновлён compare_exchange_weak до
                // актуального head — просто повторяем попытку
            }
        }

        bool pop(T& result) {
            Node* old_head{head.load(std::memory_order_acquire)};
            while (old_head != nullptr && !head.compare_exchange_weak(
                old_head,
                old_head->next,
                std::memory_order_acq_rel,
                std::memory_order_acquire)) {
                // old_head обновлён автоматически, повторяем
            }

            if (old_head == nullptr) return false;

            // (3) Здесь безопасно читать old_head->data — happens-before
            // от push() гарантирует, что данные видны
            result = std::move(old_head->data);
            delete old_head; // УПРОЩЕНИЕ: в реальности тут ABA problem +
                              // use-after-free нужно решать через hazard
                              // pointers или epoch-based reclamation
            return true;
        }

    };

    std::atomic<int> total_pushed{0};
    std::atomic<int> total_popped{0};
    std::atomic<bool> stop_consumers{false};

    void producer(const int thread_id, const int ops_count, TreiberStack<int>& stack) {
        for (int i = 0; i < ops_count; ++i) {
            // Генерируем уникальное значение: ID потока * 10000 + счетчик
            const int value = thread_id * 10000 + i;
            stack.push(value);
            total_pushed.fetch_add(1, std::memory_order_relaxed);

            // Небольшая случайная задержка увеличивает шанс пересечения потоков
            // и проявления ABA-проблемы при удалении/выделении памяти
            if (i % 10 == 0) {
                std::this_thread::yield();
            }
        }
    }

    void consumer(const int ops_count, TreiberStack<int>& stack) {
        int value;
        int local_popped = 0;

        while (local_popped < ops_count) {
            if (stack.pop(value)) {
                local_popped++;
                total_popped.fetch_add(1, std::memory_order_relaxed);
            } else {
                // Если стек пуст, даем другим потокам шанс выполнить push
                std::this_thread::yield();
            }
        }
    }
}


int main(int argc, char *argv[]) {
    std::cout << "Start to stress-test for TreiberStack...\n";

    TreiberStack<int> stack;

    constexpr int NUM_PRODUCERS = 4;
    constexpr int NUM_CONSUMERS = 4;
    constexpr int OPS_PER_THREAD = 5000; // Увеличьте это число, чтобы повысить шанс поймать ABA

    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;

    // 1. Запускаем потоки-потребители (они будут ждать данные)
    for (int i = 0; i < NUM_CONSUMERS; ++i) {
        consumers.emplace_back(consumer, OPS_PER_THREAD, std::ref(stack));
    }

    // 2. Запускаем потоки-производители
    for (int i = 0; i < NUM_PRODUCERS; ++i) {
        producers.emplace_back(producer, i, OPS_PER_THREAD, std::ref(stack));
    }

    // 3. Ожидаем завершения всех потоков
    for (auto& t : producers) {
        t.join();
    }
    for (auto& t : consumers) {
        t.join();
    }

    // 4. Проверка результатов
    std::cout << "\n--- Result ---\n";
    std::cout << "Total push:  " << total_pushed.load() << "\n";
    std::cout << "Total pop:   " << total_popped.load() << "\n";

    // Проверяем, остался ли стек пустым
    int dummy;
    if (!stack.pop(dummy)) {
        std::cout << "Stack is empty\n";
    } else {
        std::cout << "WARN: Stack is not empty.\n";
    }

    // Если числа не сходятся или программа упала с Segmentation Fault,
    // это прямое следствие ABA-проблемы и use-after-free в методе pop().
    if (total_pushed.load() != total_popped.load()) {
        std::cout << "WARN: total_popped != total_pushed\n";
    } else {
        std::cout << "total_popped == total_pushed";
    }

    return 0;
}
