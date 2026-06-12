#include <iostream>
#include <thread>
#include <mutex>

int main() {
    std::mutex mtx;
    int value;

    std::thread t1([&]() {
        // mutex is not captured yet
        std::unique_lock<std::mutex> lock(mtx, std::defer_lock);

        lock.lock();
        value = 1;
        std::cout << std::this_thread::get_id() << " set 1..." << std::endl;
        lock.unlock();

        lock.lock();
        value = 11;
        std::cout << std::this_thread::get_id() << " set 11..." << std::endl;
        lock.unlock();
    });

    std::thread t2([&]() {
        // mutex is not captured yet
        std::unique_lock<std::mutex> lock(mtx, std::defer_lock);

        lock.lock();
        value = 2;
        std::cout << std::this_thread::get_id() << " set 2..." << std::endl;
        lock.unlock();

        lock.lock();
        value = 22;
        std::cout << std::this_thread::get_id() << " set 22..." << std::endl;
        lock.unlock();
    });

    t1.join();
    t2.join();

    std::unique_lock<std::mutex> lock(mtx);
    std::cout << "result => " << value << std::endl;

    return 0;
}
