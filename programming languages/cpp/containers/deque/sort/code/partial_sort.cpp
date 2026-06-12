#include <iostream>
#include <deque>
#include <algorithm>

void _print_deque(const std::deque<int>&);

int main(int argc, char const *argv[]) {
    std::deque<int> deq {1, 10, 2, 11, 3, 12, 4};
    _print_deque(deq);

    std::partial_sort(deq.begin(), deq.begin() + 3, deq.end());
    _print_deque(deq);

    std::partial_sort(deq.begin(), deq.begin() + 5, deq.end(), [](int a, int b) {return a > b;});
    _print_deque(deq);

    return 0;
}

void _print_deque(const std::deque<int>& deque) {
    std::cout << "{";
    for (auto &item: deque) {
        std::cout << item << " ";
    }
    std::cout << "}" << std::endl;
}
