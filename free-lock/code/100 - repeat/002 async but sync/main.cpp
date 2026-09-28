#include <future>
#include <iostream>
#include <format>
#include <chrono>
#include <thread>

namespace {
    int task(const int i) {
        std::cout << std::format("[start] {}\n", std::this_thread::get_id()) << std::flush;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << std::format("[finish] {}\n", std::this_thread::get_id()) << std::flush;

        return i;
    }

    int task0() { return task(0); }
    int task1() { return task(1); }
}

int main(int argc, char *argv[]) {
    std::async(std::launch::async, [] { return task0(); });
    std::async(std::launch::async, [] { return task1(); });

    return 0;
}
