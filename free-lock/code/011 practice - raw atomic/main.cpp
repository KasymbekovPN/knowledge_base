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
