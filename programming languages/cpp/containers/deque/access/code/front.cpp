#include <iostream>
#include <deque>

template <typename T>
void print_deque(const std::deque<T>&);

int main() {
    std::deque<int> deq {1, 2, 3};
    print_deque(deq);

    std::cout << "First element: " << deq.front() << std::endl;
    print_deque(deq);

    return 0;
}

template <typename T>
void print_deque(const std::deque<T>& deque) {
    for (auto &item: deque) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}
