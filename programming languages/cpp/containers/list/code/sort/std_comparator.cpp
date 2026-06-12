#include <iostream>
#include <list>

template<typename T>
void print_list(const std::list<T>&);

int main() {
    std::list<int> numbers {11, 2, 3, 19, 3};
    print_list(numbers);

    numbers.sort(); // default, sort ascending
    print_list(numbers);

    numbers.sort(std::greater<int>()); // sort descending
    print_list(numbers);

    return 0;
}

template<typename T>
void print_list(const std::list<T>& list) {
    for (auto &item: list) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}
