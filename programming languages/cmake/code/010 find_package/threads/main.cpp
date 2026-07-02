#include <iostream>
#include <format>
#include <thread>
#include <vector>

void worker(int _id) {
    std::cout << std::format("Thread {} is working\n", _id);
}

int main() {
    std::vector<std::thread> threads;
    for (int i{}; i < 5; ++i) threads.emplace_back(worker, i);
    for (auto& t : threads) t.join();

    std::cout << "Done\n";
}
