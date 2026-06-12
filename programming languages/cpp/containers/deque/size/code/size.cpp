#include <iostream>
#include <deque>

void _print_empty(const std::deque<int>&);

int main() {
    std::deque<int> empty_deq {};
    std::deque<int> deq {1, 2, 3};

    _print_empty(empty_deq);
    _print_empty(deq);

    return 0;
}

void _print_empty(const std::deque<int>& deque) {
    std::cout
        << "Deque size is " 
        << deque.size() << std::endl;
}
