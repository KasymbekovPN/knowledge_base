#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>

constexpr size_t SIZE{5};

std::mutex mtx;
std::condition_variable cv;
std::queue<int> buffer;

void run_producer() {
    for (size_t i{}; i < SIZE; ++i) {
        std::unique_lock lock{mtx};
        buffer.push(i);
        std::cout << "produced: " << i << std::endl;
    }
    cv.notify_one();
}

void run_consumer() {
    for (size_t i{}; i < SIZE; ++i) {
        std::unique_lock lock{mtx};
        cv.wait(lock, []() { return !buffer.empty(); });
        int value = buffer.front();
        buffer.pop();
        std::cout << "consumed: " << value << std::endl;
    }
}

int main() {
    std::thread t_producer{run_producer};
    std::thread t_consumer{run_consumer};

    t_producer.join();
    t_consumer.join();

    return 0;
}
