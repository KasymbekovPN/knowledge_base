#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <vector>
#include <format>
#include <future>

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

    template<typename C>
    std::future<std::invoke_result_t<C>> submit(C&& callable) {
        using R = std::invoke_result_t<C>;
        auto task = std::make_shared<std::packaged_task<R()>>(
            std::forward<C>(callable)
        );

        auto future = task->get_future();
        {
            std::unique_lock lock{mtx};
            queue.push([task]() {
                (*task)();
            });
        }

        cv.notify_one();

        return future;
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

    std::vector<std::future<int>> futures;
    for (int i{}; i < 10; ++i) {
        futures.emplace_back(pool.submit([i]()-> int {
            std::cout << std::format(
                "Task {} in thread {}\n",
                i,
                std::this_thread::get_id()
            );
            return i*i;
        }));
    }

    for (auto &&future: futures) {
        std::cout << std::format("! {}\n", future.get());
    }

    return 0;
}
