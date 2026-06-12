#include <iostream>
#include <list>

template<typename T>
void print_list(const std::list<T>&);

int main() {
    std::list<int> numbers {1, 2, 3, 4, 5, 6, 7};
    print_list(numbers);

    auto single_it = numbers.begin();
    std::advance(single_it, 2);
    numbers.erase(single_it);
    print_list(numbers);

    auto start_it = numbers.begin();
    std::advance(start_it, 1);
    auto finish_it = numbers.begin();
    std::advance(finish_it, 3);
    numbers.erase(start_it, finish_it);
    print_list(numbers);

    return 0;
}


template<typename T>
void print_list(const std::list<T>& list) {
    for (const auto &item: list) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}
