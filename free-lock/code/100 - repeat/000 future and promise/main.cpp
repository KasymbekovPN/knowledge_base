#include <iostream>
#include <future>
#include <thread>
#include <chrono>

namespace {
    long compute() {
        const auto raw{std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count()};
        const auto value{raw % 4};
        if (value == 0) {
            throw std::runtime_error{"zero error"};
        }

        return value;
    }

    void worker(std::promise<long> promise) {
        try {
            promise.set_value(compute());
        } catch(...) {
            promise.set_exception(std::current_exception());
        }
    }
}

int main(int argc, char *argv[]) {
    std::promise<long> p;
    std::future<long> f{p.get_future()};

    std::thread t{worker, std::move(p)};
    std::cout << std::format("result: {}\n", f.get()) << std::flush;

    t.join();

    return 0;
}
