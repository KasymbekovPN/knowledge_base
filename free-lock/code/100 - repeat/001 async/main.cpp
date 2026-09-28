#include <future>
#include <iostream>
#include <format>
#include <thread>
#include <chrono>

namespace {
    int heavy(const int i) {
        std::cout << std::format("[start] {}\n", std::this_thread::get_id()) << std::flush;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << std::format("[start] {}\n", std::this_thread::get_id()) << std::flush;

        return i * i * i;
    }
}

int main(int argc, char *argv[]) {
    auto f0{std::async(std::launch::async, [] { return heavy(2); })};
    auto f1{std::async(std::launch::async, [] { return heavy(3); })};

    const auto v0{f0.get()};
    const auto v1{f1.get()};
    std::cout << std::format("{} + {} = {}\n", v0, v1, v0 + v1);

    return 0;
}
