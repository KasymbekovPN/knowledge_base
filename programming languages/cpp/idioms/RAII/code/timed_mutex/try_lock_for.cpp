#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

std::timed_mutex tmx;

void worker(int);

int main() {
    // tmx.lock();

    std::thread t1(worker, 1);
    std::thread t2(worker, 2);

    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    tmx.unlock();

    t1.join();
    t2.join();

    return 0;
}

void worker(int id) {
    if (tmx.try_lock_for(std::chrono::milliseconds(100))) {
        std::cout << "Worker " << id << " got the lock" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        tmx.unlock();
    } else {
        std::cout << "Worker " << id << " timed out" << std::endl;
    }
}
