#include <iostream>
#include <string>
#include <deque>

void _print_deque(const std::deque<int>&);

int main(int argc, char const *argv[]) {
    std::deque<int> deq {1, 2, 3};
    _print_deque(deq);

    deq.push_back(42);
    _print_deque(deq);

    return 0;
}

void _print_deque(const std::deque<int>& deque) {
    for (auto &item: deque) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}
