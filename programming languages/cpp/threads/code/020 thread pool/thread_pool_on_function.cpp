#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <vector>
#include <format>

class ThreadPool {

public:
    explicit ThreadPool(const size_t num_threads) {
        for (size_t i{}; i < num_threads; ++i) {
            workers.emplace_back([this](){
                loop();
            });
        }
    }

    ~ThreadPool() {
        {
            std::unique_lock lock{mtx};
            stop = true;
        }
        cv.notify_all();
        for (auto &worker: workers) {
            worker.join();
        }
    }

    void submit(std::function<void()> task) {
        {
            std::unique_lock lock{mtx};
            queue.push(std::move(task));
        }
        cv.notify_one();
    }

private:
    void loop() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock lock{mtx};
                cv.wait(lock, [this]() {
                    return stop || !queue.empty();
                });

                if (stop && queue.empty()) {
                    return;
                }

                task = std::move(queue.front());
                queue.pop();
            }
            task();
        }
    }
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> queue;
    std::mutex mtx;
    std::condition_variable cv;
    bool stop{};
};

int main() {
    ThreadPool pool{4};

    for (size_t i{}; i < 10; ++i) {
        pool.submit([i]() {
            std::cout << std::format(
                "Task {} in thread {}\n",
                i,
                std::this_thread::get_id()
            );
        });
    }

    return 0;
}
