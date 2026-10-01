#include <iostream>
#include <chrono>
#include <atomic>
#include <thread>
#include <condition_variable>
#include <functional>
#include <map>
#include <vector>
#include <mutex>

namespace {
    using Clock = std::chrono::steady_clock;
    using TimePoint = std::chrono::time_point<Clock>;
    using ms = std::chrono::milliseconds;

    class CallbackScheduler {
        std::condition_variable wakeup_cv_;
        std::atomic<bool> wakeup_{false};
        std::atomic<bool> running_{true};
        std::mutex mtx_;
        std::thread support_thread_;
        std::map<TimePoint, std::vector<std::function<void()>>> storage_;

    public:
        explicit CallbackScheduler() {
            support_thread_ = std::thread([&] {
                while (running_.load(std::memory_order_acquire)) {
                    std::unique_lock<std::mutex> lk(mtx_);
                    const auto it{storage_.begin()};
                    if (it == storage_.end()) {
                        std::this_thread::yield();
                        continue;
                    }

                    if (const auto now{Clock::now()};
                        now >= it->first) {
                        for (auto& f: it->second) f();
                        storage_.erase(it);
                    } else {
                        std::this_thread::yield();
                        wakeup_cv_.wait_for(lk, it->first - now, [&] {
                            if (!running_.load(std::memory_order_acquire)) {
                                return true;
                            }

                            auto old_value{wakeup_.load(std::memory_order_release)};
                            if (!old_value) return old_value;

                            while (!wakeup_.compare_exchange_weak(
                                old_value,
                                false,
                                std::memory_order_acq_rel,
                                std::memory_order_release)) {}

                            return true;
                        });
                    }
                }
            });
        }

        ~CallbackScheduler() {
            running_.store(false, std::memory_order_release);
            wakeup_cv_.notify_all();
            support_thread_.join();
        }

        [[nodiscard]] int Schedule(std::function<void()> callback, TimePoint when) {
            const auto now{Clock::now()};
            if (now >= when) {
                return -1;
            }

            std::lock_guard<std::mutex> lk(mtx_);

            if (!storage_.empty() && now < storage_.begin()->first) {
                wakeup_.store(true, std::memory_order_release);
                wakeup_cv_.notify_one();
            }

            if (storage_.contains(when)) {
                storage_[when].push_back(std::move(callback));
            } else {
                storage_.emplace(when, std::vector<std::function<void()>>{std::move(callback)});
            }

            return 0;
        }
    };

    int g_pass = 0, g_fail = 0;
    void check(bool cond, const std::string& msg) {
        if (cond) { std::cout << "  OK   " << msg << "\n"; ++g_pass; }
        else      { std::cout << "  FAIL " << msg << "\n"; ++g_fail; }
    }

    void test_simple() {
        std::mutex m;
        std::condition_variable cv;
        bool done{false};

        auto callback = [&] {
            {
                std::lock_guard<std::mutex> lk(m);
                done = true;
            }
            cv.notify_one();
        };

        CallbackScheduler scheduler;
        const auto when{Clock::now() + ms(10)};
        const auto r{scheduler.Schedule(callback, when)};
        (void)r;

        {
            std::unique_lock<std::mutex> lk(m);
            cv.wait_for(lk, ms(20), [&] { return done; });
        }
        check(done, "simple test");
    }

    // 1. Базовый случай: один callback должен сработать
    void test_single_callback_fires() {
        std::cout << "[1] test_single_callback_fires\n";
        CallbackScheduler s;
        std::atomic<bool> fired{false};
        int r = s.Schedule([&] { fired.store(true, std::memory_order_release); },
                            Clock::now() + ms(20));
        check(r == 0, "Schedule() вернул успех (0)");
        auto deadline = Clock::now() + ms(500);
        while (!fired.load(std::memory_order_acquire) && Clock::now() < deadline)
            std::this_thread::sleep_for(ms(5));
        check(fired.load(), "callback реально сработал в пределах 500мс");
    }

    // 2. Callback НЕ должен срабатывать раньше своего времени
    void test_callback_does_not_fire_early() {
        std::cout << "[2] test_callback_does_not_fire_early\n";
        CallbackScheduler s;
        std::atomic<bool> fired{false};
        auto when = Clock::now() + ms(100);
        s.Schedule([&] { fired.store(true, std::memory_order_release); }, when);
        std::this_thread::sleep_for(ms(30));
        check(!fired.load(), "callback НЕ сработал преждевременно (через 30мс из 100мс)");
    }

    // 3. Несколько callback'ов на разное время -- порядок срабатывания
    void test_multiple_callbacks_fire_in_order() {
        std::cout << "[3] test_multiple_callbacks_fire_in_order\n";
        CallbackScheduler s;
        std::mutex order_mtx;
        std::vector<int> fire_order;
        auto base = Clock::now();
        s.Schedule([&] { std::lock_guard<std::mutex> lk(order_mtx); fire_order.push_back(3); }, base + ms(60));
        s.Schedule([&] { std::lock_guard<std::mutex> lk(order_mtx); fire_order.push_back(1); }, base + ms(20));
        s.Schedule([&] { std::lock_guard<std::mutex> lk(order_mtx); fire_order.push_back(2); }, base + ms(40));
        auto deadline = Clock::now() + ms(500);
        while (Clock::now() < deadline) {
            std::lock_guard<std::mutex> lk(order_mtx);
            if (fire_order.size() == 3) break;
            std::this_thread::sleep_for(ms(5));
        }
        std::lock_guard<std::mutex> lk(order_mtx);
        check(fire_order.size() == 3, "все три callback'а сработали");
        check(fire_order == std::vector<int>({1, 2, 3}), "сработали в правильном порядке (1,2,3)");
    }

    // 4. Несколько callback'ов на ОДНО время -- все должны выполниться
    void test_multiple_callbacks_same_time() {
        std::cout << "[4] test_multiple_callbacks_same_time\n";
        CallbackScheduler s;
        std::atomic<int> counter{0};
        const auto when = Clock::now() + ms(30);
        for (int i = 0; i < 5; ++i) {
            const auto r{s.Schedule([&] { counter.fetch_add(1, std::memory_order_relaxed); }, when)};
            (void)r;
        }

        const auto deadline = Clock::now() + ms(500);
        while (counter.load() < 5 && Clock::now() < deadline)
            std::this_thread::sleep_for(ms(5));
        check(counter.load() == 5, "все 5 callback'ов на одно время сработали");
    }

    // 5. Schedule() с временем в прошлом -- должен вернуть -1 и не выполниться
    void test_schedule_in_past_rejected() {
        std::cout << "[5] test_schedule_in_past_rejected\n";
        CallbackScheduler s;
        std::atomic<bool> fired{false};
        int r = s.Schedule([&] { fired.store(true); }, Clock::now() - ms(10));
        check(r == -1, "Schedule() с временем в прошлом вернул -1");
        std::this_thread::sleep_for(ms(100));
        check(!fired.load(), "callback из прошлого НЕ был выполнен");
    }

    // 6. КРИТИЧНО: добавление БЛИЖНЕГО callback'а поверх ДАЛЁКОГО должно
    //    разбудить спящий поток, а не ждать естественного пробуждения
    void test_scheduling_earlier_callback_wakes_thread() {
        std::cout << "[6] test_scheduling_earlier_callback_wakes_thread\n";
        CallbackScheduler s;
        std::atomic<bool> early_fired{false};
        std::atomic<bool> late_fired{false};
        auto r =s.Schedule([&] { late_fired.store(true); }, Clock::now() + ms(600));
        const auto early_scheduled_at = Clock::now();
        r = s.Schedule([&] { early_fired.store(true); }, early_scheduled_at + ms(50));
        (void)r;
        const auto deadline = Clock::now() + ms(300);
        while (!early_fired.load() && Clock::now() < deadline)
            std::this_thread::sleep_for(ms(5));
        check(early_fired.load(), "ближний callback сработал БЫСТРО, не дожидаясь дальнего");
        check(!late_fired.load(), "дальний callback ещё НЕ сработал к этому моменту");
    }

    // 7. Массовая нагрузка -- все callback'и должны сработать
    void test_many_callbacks_all_fire() {
        std::cout << "[7] test_many_callbacks_all_fire\n";
        CallbackScheduler s;
        constexpr int N = 50;
        std::atomic<int> counter{0};
        const auto base = Clock::now();
        for (int i = 0; i < N; ++i) {
            const auto r = s.Schedule([&] { counter.fetch_add(1, std::memory_order_relaxed); },
                        base + ms(10 + i * 3));
            (void)r;
        }

        const auto deadline = Clock::now() + ms(2000);
        while (counter.load() < N && Clock::now() < deadline)
            std::this_thread::sleep_for(ms(10));
        check(counter.load() == N, "все " + std::to_string(N) + " callback'ов сработали");
    }

    // 8. Конкурентные Schedule() из нескольких потоков
    void test_concurrent_schedule_calls() {
        std::cout << "[8] test_concurrent_schedule_calls\n";
         CallbackScheduler s;
        constexpr int THREADS = 4;
        constexpr int PER_THREAD = 20;
        std::atomic<int> counter{0};
        std::atomic<int> rejected{0};
        std::vector<std::thread> producers;
        const auto base = Clock::now();
        for (int t = 0; t < THREADS; ++t) {
            producers.emplace_back([&, t] {
                for (int i = 0; i < PER_THREAD; ++i) {
                    const auto r{s.Schedule(
                    [&] { counter.fetch_add(1, std::memory_order_relaxed);},
                    base + ms(2 + (t * PER_THREAD + i)))
                    };
                    if (r == -1) rejected.fetch_add(1, std::memory_order_relaxed);
                }
            });
        }

        for (auto& th : producers) th.join();

        const auto deadline = Clock::now() + ms(10'000);
        while (counter.load(std::memory_order_relaxed) + rejected.load(std::memory_order_relaxed) < THREADS * PER_THREAD && Clock::now() < deadline)
            std::this_thread::sleep_for(ms(20));
        const auto callback_executed{counter.load(std::memory_order_relaxed)};
        std::cout << std::format("callback_executed: {}\n", callback_executed) << std::flush;
        std::cout << std::format("rejected: {}\n", rejected.load(std::memory_order_relaxed)) << std::flush;
        check(callback_executed + rejected == THREADS * PER_THREAD,
              "все callback'и от " + std::to_string(THREADS) + " потоков сработали");
    }

    // 9. Деструктор должен завершиться быстро, даже если есть отложенные callback'и
    void test_destructor_stops_cleanly_with_pending_callbacks() {
        std::cout << "[9] test_destructor_stops_cleanly_with_pending_callbacks\n";
        const auto start = Clock::now();
        {
            CallbackScheduler s;
            const auto r = s.Schedule([] {}, Clock::now() + ms(10'000));
            (void)r;
        }
        const auto elapsed = std::chrono::duration_cast<ms>(Clock::now() - start).count();
        constexpr int max_elapsed{10};
        check(elapsed < max_elapsed, "деструктор завершился быстро (" + std::to_string(elapsed) + "мс)");
    }

    // 10. Пустой планировщик -- без единого Schedule() -- не должен падать
    void test_empty_scheduler_no_crash() {
        std::cout << "[10] test_empty_scheduler_no_crash (гонять под ASan!)\n";
        for (int i = 0; i < 20; ++i) {
            CallbackScheduler s;
            std::this_thread::sleep_for(ms(2));
        }
        check(true, "20x создание/уничтожение пустого планировщика без крашей");
    }
}

int main() {
    test_simple();
    test_single_callback_fires();
    test_callback_does_not_fire_early();
    test_multiple_callbacks_fire_in_order();
    test_multiple_callbacks_same_time();
    test_schedule_in_past_rejected();
    test_scheduling_earlier_callback_wakes_thread();
    test_many_callbacks_all_fire();
    test_concurrent_schedule_calls();
    test_destructor_stops_cleanly_with_pending_callbacks();
    test_empty_scheduler_no_crash();

    std::cout << "\n===== ИТОГО: " << g_pass << " passed, " << g_fail << " failed =====\n";
    return g_fail == 0 ? 0 : 1;
}
