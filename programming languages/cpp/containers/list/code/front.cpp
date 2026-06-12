#include <iostream>
#include <list>

template<typename T>
void print_list(const std::list<T>&);

int main() {
    std::list<int> numbers {1, 2, 3};
    print_list(numbers);

    std::cout << "first <= " << numbers.front() << std::endl;
    print_list(numbers);

    return 0;
}


template<typename T>
void print_list(const std::list<T>& list) {
    for (auto &&item: list) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}
