```cpp
#include <algorithm>
#include <cassert>
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>
#include <array>
#include <mutex>
#include <unordered_map>

namespace {
    template <typename T>
    class CustomStack {

    private:
        struct Node {
            T value;
            Node* next;
            explicit Node(T value) : value(std::move(value)), next(nullptr) {}
        };

        class HazardControl {

            constexpr static int HAZARDOUS_SIZE{128};
            struct Slot {
                // слот закреплён за каким-то потоком навсегда
                std::atomic<bool> in_use{false};
                // что этот поток сейчас защищает (или nullptr)
                std::atomic<Node*> ptr{nullptr};
            };

            std::array<Slot, HAZARDOUS_SIZE> slots_{};

            // Закрепление слота происходит РОВНО ОДИН РАЗ за жизнь потока (на инстанс
            // HazardControl), дальше — просто переиспользуется. thread_local кэш
            // на случай нескольких CustomStack-инстансов в одной программе.
            Slot& get_thread_slot() {
                thread_local std::unordered_map<const HazardControl*, Slot*> cache;
                if (auto it{cache.find(this)}; it != cache.end()) return *it->second;

                for (auto& s: slots_) {
                    if (bool expected{false};
                        s.in_use.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
                        cache[this] = &s;
                        return s;
                    }
                }

                throw std::runtime_error{"hazard slot pool exhausted"};
            }

        public:
            void protect(Node* node) {
                get_thread_slot().ptr.store(node, std::memory_order_release);
            }

            void clear() {
                get_thread_slot().ptr.store(nullptr, std::memory_order_release);
            }

            // читающий скан по всем слотам — никогда не пишет, поэтому не мешает
            // "быстрому пути" других потоков и не требует CAS
            bool is_protected(const Node* node) const {
                return std::ranges::any_of(slots_, [&node](const Slot& s) {
                    if (s.in_use.load(std::memory_order_acquire) &&
                        s.ptr.load(std::memory_order_acquire) == node) {
                        return true;
                    }
                    return false;
                });
            }
        };

    public:
        explicit CustomStack() = default;

        ~CustomStack() {
            const Node* node{head_.load(std::memory_order_relaxed)};
            delete_nodes_on_dtor(node);

            for (auto& p: retired_list_) delete p;
            retired_list_.clear();
        }

        bool push(T value) {
            Node* new_node{new Node{std::move(value)}};
            Node* head_node{head_.load(std::memory_order_relaxed)};
            new_node->next = head_node;

            while (!head_.compare_exchange_weak(
                head_node,
                new_node,
                std::memory_order_acq_rel,
                std::memory_order_relaxed)) {

                new_node->next = head_node;
            }

            return true;
        }

        bool pop(T& result) {
            Node* old_head{head_.load(std::memory_order_relaxed)};
            while (old_head != nullptr) {
                hazard_control_.protect(old_head);

                // старый узел мог быть уже вынут (и потенциально удалён) между
                // первым load() и store() в hazard-слот — перепроверяем
                if (head_.load(std::memory_order_acquire) != old_head) {
                    old_head = head_.load(std::memory_order_relaxed);
                    continue;
                }

                if (Node* new_head{old_head->next};
                    head_.compare_exchange_weak(
                    old_head,
                    new_head,
                    std::memory_order_acq_rel,
                    std::memory_order_acquire)) {

                    result = std::move(old_head->value);
                    hazard_control_.clear();
                    retire(old_head);

                    return true;
                }
            }

            hazard_control_.clear();

            return false;
        }

    private:
        void delete_nodes_on_dtor(const Node* node) {
            if (!node) return;
            delete_nodes_on_dtor(node->next);

            delete node;
        };

        void retire(Node* p) {
            if (hazard_control_.is_protected(p)) {
                std::lock_guard<std::mutex> guard{retired_list_mtx_};
                retired_list_.push_back(p);
            } else {
                delete p;
            }
            reclaim_some();
        }

        void reclaim_some() {
            if (const auto current_counter{retired_list_counter_.fetch_add(1, std::memory_order_relaxed)};
                current_counter < RETIRED_LIST_TRIGGER_SIZE) {
                return;
            }

            retired_list_counter_.store(0, std::memory_order_relaxed);

            std::lock_guard<std::mutex> guard{retired_list_mtx_};
            auto it{retired_list_.begin()};
            while (it != retired_list_.end()) {
                if (hazard_control_.is_protected(*it)) {
                    ++it;
                } else {
                    delete *it;
                    it = retired_list_.erase(it);
                }
            }
        }

        std::atomic<Node*> head_{nullptr};
        HazardControl hazard_control_;

        static constexpr int RETIRED_LIST_TRIGGER_SIZE{100};
        std::atomic<int> retired_list_counter_{0};
        std::mutex retired_list_mtx_;
        std::list<Node*> retired_list_;
    };

    void test_simple(const int num_producers,
                     const int iter_per_producer,
                     const int num_consumers,
                     const int iter_per_consumer) {

        const auto start{std::chrono::steady_clock::now()};

        int total{num_producers * iter_per_producer};
        std::atomic<long long> checksum_in{0};
        std::atomic<long long> checksum_out{0};
        std::atomic<int> produced{0};
        std::atomic<int> consumed{0};

        CustomStack<int> stack;
        std::vector<std::thread> producers;
        for (int p{}; p < num_producers; ++p) {
            producers.emplace_back([p, &stack, &checksum_in, &produced, &iter_per_producer]() {
                for (int i{}; i < iter_per_producer; ++i) {
                    const int value{p * iter_per_producer + i};
                    stack.push(value);
                    checksum_in.fetch_add(value, std::memory_order_relaxed);
                    produced.fetch_add(1, std::memory_order_relaxed);
                }
            });
        }

        std::vector<std::thread> consumers;
        for (int c{}; c < num_consumers; ++c) {
            consumers.emplace_back([&stack, &checksum_out, &consumed, &iter_per_consumer, &total]() {
                for (int i{}; i < iter_per_consumer; ++i) {
                    int value;
                    while (consumed.load(std::memory_order_relaxed) < total) {
                        if (stack.pop(value)) {
                            checksum_out.fetch_add(value, std::memory_order_relaxed);
                            consumed.fetch_add(1, std::memory_order_relaxed);
                        } else {
                            std::this_thread::yield();
                        }
                    }
                }
            });
        }

        for (auto& t: producers) t.join();
        for (auto& t: consumers) t.join();

        const auto ms{std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start).count()};

        const auto checksum_in_value{checksum_in.load()};
        const auto checksum_out_value{checksum_out.load()};
        const auto produced_value{produced.load()};
        const auto consumed_value{consumed.load()};

        const bool ok{
            checksum_in_value == checksum_out_value &&
                produced_value == total &&
                    consumed_value == total
        };

        std::cout << std::format("[test_simple] {}, in= {}, out= {}, {} ms\n",
            ok ? "PASSED" : "FAILED",
            checksum_in_value,
            checksum_out_value,
            ms);

        assert(ok && "checksum mismatch");
    }
}

int main(int argc, char *argv[]) {
    constexpr int T0_NUM_PRODUCERS{50};
    constexpr int T0_ITER_PER_PRODUCERS{50};
    constexpr int T0_NUM_CONSUMERS{50};
    constexpr int T0_ITER_PER_CONSUMERS{50};

    test_simple(T0_NUM_PRODUCERS, T0_ITER_PER_PRODUCERS, T0_NUM_CONSUMERS, T0_ITER_PER_CONSUMERS);

    return 0;
}

```

### Реализовать самому на raw atomics (не подглядывая)

Да, именно так — это пункт из **практики** Блока 2, где нужно самому написать структуру данных на `std::atomic`, не подглядывая в реализации, которые уже разбирали в этом чате. Смысл упражнения — не в том, чтобы получить готовый код, а в том, чтобы вы сами прошли через все ловушки (memory_order, ABA, порядок операций), которые я до этого объяснял на пальцах — тогда они закрепятся, а не останутся "я вроде понял, когда Claude объяснял".

## Что конкретно делать

**Возьмите одну структуру за раз**, начиная с самой простой из списка Блока 2:

1. **SPSC ring buffer** — самое простое, с него и стоит начать
2. Treiber stack
3. MPSC-очередь

Не пытайтесь сразу воспроизвести MPMC Vyukov'а — это самая сложная из показанных, и на ней легче всего скопировать структуру мышления вместо того, чтобы понять её самостоятельно.

## Правила "не подглядывая"

- Закройте эту переписку (или хотя бы не листайте её во время написания)
- Дайте себе конкретное время — например, 45 минут на SPSC ring buffer
- Разрешено: cppreference (описание `std::atomic`, `memory_order`), но не примеры чужих реализаций lock-free структур
- Не разрешено: подсматривать в код, который я писал выше в этом чате, в boost::lockfree, в готовые GitHub-репозитории

## Как понять, что получилось

Ваша реализация должна пройти три проверки — это и есть критерий "работает", а не просто "компилируется":

1. **Функциональный тест**: push N значений одним потоком, pop N значений другим, checksum до и после совпадает
2. **ThreadSanitizer**: `g++ -fsanitize=thread -pthread` — должен быть чист от гонок
3. **Стресс-тест**: не 10 значений, а хотя бы 100 000-1 000 000 — многие баги в lock-free коде не проявляются на маленьких объёмах данных, потому что нужный interleaving потоков просто не успевает произойти

## Что делать, если застряли

Если через 45-60 минут не получается сдвинуться — это нормально, lock-free код сложен именно тем, что интуиция из обычного многопоточного программирования (мьютексы) здесь не работает напрямую. В этом случае:

- Напишите, на каком конкретно месте застряли (например: "не понимаю, что делать в pop, если producer ещё не успел обновить tail") — так я смогу подсказать конкретную точку, а не выдать готовое решение
- Или пришлите свою реализацию, и я укажу баги/гонки без переписывания её за вас — это ближе к code review, чем к готовому ответу, и полезнее для собеседования
