#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

void test(int, int);

std::recursive_timed_mutex mtx;

int main() {
    std::thread t1(test, 1, 1);
    std::thread t2(test, 1, 2);

    t1.join();
    t2.join();

    return 0;
}


void test(int _depth, int _id) {
    if (!mtx.try_lock_for(std::chrono::milliseconds(200))) {
        std::cout << "[" << _id << "] Timeout at depth " << _depth  << std::endl;
        return;
    }

    std::cout << "[" << _id << "] Locked at depth " << _depth << std::endl;
    if (_depth < 3) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        test(_depth + 1, _id);
    }

    mtx.unlock();
}
