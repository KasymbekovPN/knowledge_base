#include <iostream>
#include <deque>
#include <algorithm>
#include <ranges>

void _print_deq(const std::deque<int>&);

int main() {
    std::deque<int> deq = {5, 3, 1, 4, 2};
    _print_deq(deq);

    std::ranges::sort(deq | std::views::take(3));
    _print_deq(deq);

    std::ranges::sort(deq, std::less<int>());
    _print_deq(deq);

    return 0;
}


void _print_deq(const std::deque<int>& deque) {
    for (const auto &item: deque) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}
