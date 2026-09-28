#include <iostream>
#include <format>
#include <thread>
#include <future>

namespace {
    void task(std::promise<int> p) {
        p.set_value(42);
    }
}

int main(int argc, char *argv[]) {
    std::promise<int> p;
    auto sf0 = p.get_future().share();

    std::thread t0{[&sf0] { std::cout << std::format("t0:: {}\n", sf0.get()) << std::flush; }};
    std::thread t1{[&sf0] { std::cout << std::format("t1:: {}\n", sf0.get()) << std::flush; }};
    std::thread t2{task, std::move(p)};

    t0.join();
    t1.join();
    t2.join();

    return 0;
}
