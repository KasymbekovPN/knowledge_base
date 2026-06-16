#include <iostream>
#include <format>
#include <string>
#include <chrono>
#include <queue>
#include <utility>
#include <thread>
#include <coroutine>
#include <condition_variable>
#include <mutex>

class Scheduler {
public:
    using Handle = std::coroutine_handle<>;
    using Clock = std::chrono::steady_clock;

private:
    std::mutex mtx;
    std::condition_variable cv;
    bool stop_flag{false};

    std::queue<Handle> ready;
    struct Timer {
        Clock::time_point when;
        Handle handle;
        bool operator<(const Timer& _other) const {
            return when > _other.when;
        }
    };
    std::priority_queue<Timer> timers;

    bool hasWork() const {
        return !ready.empty() || !timers.empty();
    }

public:
    struct YieldAwaiter {
        Scheduler& s;
        bool await_ready() const noexcept { return false; }
        void await_suspend(Handle _handle) const noexcept {
            s.scheduleReady(_handle);
        }
        void await_resume() const noexcept {}
    };


    YieldAwaiter yield() { return YieldAwaiter{*this}; }

    template <typename R, typename P>
    auto sleep_for(std::chrono::duration<R, P> _duration) {
        auto&& dd = std::chrono::duration_cast<Clock::duration>(_duration);
        struct Awaiter {
            Scheduler& s;
            Clock::duration duration;
            bool await_ready() const noexcept { return duration <= Clock::duration::zero(); }
            void await_suspend(Handle _h) const noexcept {
                s.scheduleTimer(Clock::now() + duration, _h);
            }
            void await_resume() const noexcept {}
        };
        return Awaiter{*this, dd};
    }

    void schedule(Handle _h) {
        scheduleReady(_h);
    }

    void run() {
        std::unique_lock lock{mtx};
        while (true) {
            if (!ready.empty()) {
                Handle h = ready.front();
                ready.pop();
                lock.unlock();

                h.resume();
                if (h.done()) h.destroy(); 

                lock.lock();
                continue;
            }

            if (!timers.empty()) {
                auto when = timers.top().when;
                cv.wait_until(lock, when, [&]() {
                    return stop_flag || !ready.empty();
                });

                if (stop_flag) break;

                auto now = Clock::now();
                while (!timers.empty() && timers.top().when <= now) {
                    ready.push(timers.top().handle);
                    timers.pop();
                }
                continue;
            }

            cv.wait(lock, [&]() { return stop_flag || hasWork(); });
            if (stop_flag && !hasWork()) break;
        }
    }

    void stop() {
        {
            std::lock_guard lock{mtx};
            stop_flag = true;
        }
        cv.notify_one();
    }

private:
    void scheduleReady(Handle _h) {
        {
            std::lock_guard lock{mtx};
            ready.push(_h);
        }
        cv.notify_one();
    }
    void scheduleTimer(Clock::time_point _when, Handle _h) {
        {
            std::lock_guard lock{mtx};
            timers.push(Timer{_when, _h});
        }
        cv.notify_one();
    }
};

struct Task {
    struct promise_type {
        Task get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };

    std::coroutine_handle<promise_type> h;

    explicit Task(std::coroutine_handle<promise_type> _handle): h{_handle} {}
    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;
    Task(Task&& _other) noexcept: h{std::exchange(_other.h, {})} {}
};

Scheduler scheduler;
using namespace std::chrono_literals;

Task ticker(std::string _name, int _ticks, std::chrono::milliseconds _period) {
    for (int i{}; i < _ticks; ++i) {
        std::cout << std::format(
            "{} tick {} @ {}ms\n",
            _name,
            i,
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()
            ).count() % 100000
        );
        co_await scheduler.sleep_for(_period);
    }
    std::cout << std::format("{} finished\n", _name);

    co_return;
}

Task latecomer() {
    std::cout << ">>> latecomer woke the loop and ran!\n";
    co_return;
}

int main() {
    // Длинный таймер: loop уснёт на ~300мс
    Task t = ticker("slow", 10, 300ms);
    scheduler.schedule(t.h);

    // Внешний поток через 50мс подкинет задачу — loop должен проснуться РАНЬШЕ 300мс
    std::thread external_thread{[]{
        std::this_thread::sleep_for(50ms);
        std::cout << "[external thread] injecting a task at ~50ms\n";
        Task late = latecomer();
        scheduler.schedule(late.h);
        late.h = {};
    }};

    // Завершим loop, когда вся работа отыграет: дадим времени и попросим stop
    std::thread stopper_thread{[] {
        std::this_thread::sleep_for(1500ms);
        scheduler.stop();
    }};

    scheduler.run();

    external_thread.join();
    stopper_thread.join();

    std::cout << "done\n";

    return 0;
}
