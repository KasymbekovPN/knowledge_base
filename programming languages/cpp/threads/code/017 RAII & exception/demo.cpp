#include <iostream>
#include <thread>
#include <stdexcept>
#include <future>

void worker(std::promise<void> _promise) {
    try {
        throw std::runtime_error("fail in thread");
    } catch (...) {
        _promise.set_exception(std::current_exception());
    }
}

int main() {
    std::promise<void> p;
    auto f0 = p.get_future();
    std::thread t0{worker, std::move(p)};
    t0.join();

    try {
        f0.get();
    } catch (const std::exception& e) {
        std::cerr << "[0] caught: " << e.what() << '\n';
    }

    auto f1 = std::async(std::launch::async, []() {
        throw std::runtime_error("fail");
    });
    try {
        f1.get();
    } catch (const std::exception& e) {
        std::cerr << "[1] caught: " << e.what() << '\n';
    }

    return 0;
}
