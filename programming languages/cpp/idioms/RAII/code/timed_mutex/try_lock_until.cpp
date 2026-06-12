#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

std::timed_mutex tmx;

void worker(int, int);

int main() {
    std::thread t1(worker, 1, 200);
    std::thread t2(worker, 2, 300);

    t1.join();
    t2.join();

    return 0;
}

void worker(int id, int ms) {
    auto deadline =
        std::chrono::steady_clock::now() +
        std::chrono::milliseconds(ms);

    if (tmx.try_lock_until(deadline)) {
        std::cout << "Worker " << id << " got the lock" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        tmx.unlock();
    } else {
        std::cout << "Worker " << id << " timed out" << std::endl;
    }
}
