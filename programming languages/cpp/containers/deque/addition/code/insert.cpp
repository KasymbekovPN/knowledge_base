#include <iostream>
#include <string>
#include <deque>

void _print_deque(const std::deque<int>&);

int main(int argc, char const *argv[]) {
    std::deque<int> deq {1, 2, 3};
    _print_deque(deq);

    auto it = deq.begin();
    std::advance(it, 1);
    deq.insert(it, 101);
    _print_deque(deq);

    deq.insert(deq.end(), 102);
    _print_deque(deq);

    // usage invalid iterator
    // it = deq.end();
    // std::advance(it, 10);
    // deq.insert(it, 103);
    // print_deque(deq);

    return 0;
}

void _print_deque(const std::deque<int>& deque) {
    for (auto &item: deque) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}
