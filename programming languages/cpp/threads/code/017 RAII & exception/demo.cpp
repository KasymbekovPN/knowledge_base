#include <iostream>
#include <thread>
#include <exception>
#include <future>

std::exception_ptr thread_exception;

void worker() {
    try {
        throw std::runtime_error("fail in thread");
    } catch(...) {
        thread_exception = std::current_exception();
    }
}

int main() {
    std::thread t0{worker};
    t0.join();

    if (thread_exception) {
        try {
            std::rethrow_exception(thread_exception);
        } catch(const std::exception& e) {
            std::cerr << "[0] caught: " << e.what() << '\n';
        }
    }

    auto&& f = std::async(std::launch::async, []() -> int {
        throw std::runtime_error("fail");
    });
    try {
        f.get();
    } catch(const std::exception& e) {
        std::cerr << "[1] caught: " << e.what() << '\n';
    }

    return 0;
}
