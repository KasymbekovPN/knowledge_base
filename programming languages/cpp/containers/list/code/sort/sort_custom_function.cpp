#include <iostream>
#include <list>

template<typename T>
void print_list(const std::list<T>&);

bool compare_ascending(int, int);
bool compare_descending(int, int);

int main() {
    std::list<int> numbers {11, 2, 3, 19, 3};
    print_list(numbers);

    numbers.sort(compare_descending);
    print_list(numbers);

    numbers.sort(compare_ascending);
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

bool compare_ascending(int a, int b) {
    return a < b;
}

bool compare_descending(int a, int b) {
    return a > b;
}
