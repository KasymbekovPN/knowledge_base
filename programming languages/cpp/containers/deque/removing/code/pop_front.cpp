#include <iostream>
#include <string>
#include <deque>

void _print_deque(const std::deque<int>&);
void _pop_front(std::deque<int>&);

int main(int argc, char const *argv[]) {
    std::deque<int> deq {1, 2};
    _print_deque(deq);

    _pop_front(deq);
    _print_deque(deq);

    _pop_front(deq);
    _print_deque(deq);

    _pop_front(deq);
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

void _pop_front(std::deque<int>& deque) {
    if (!deque.empty()) {
        deque.pop_front();
    } else {
        std::cout << "Empty!" << std::endl;
    }
}
