#include <iostream>
#include <forward_list>

template<typename T>
void print_flist(const std::forward_list<T>&);

bool compare_descending(int, int);
bool compare_ascending(int, int);

int main() {
    std::forward_list<int> numbers {123, 17, 200, 1, -1};
    print_flist(numbers);

    numbers.sort(compare_ascending);
    print_flist(numbers);

    numbers.sort(compare_descending);
    print_flist(numbers);

    return 0;
}

template<typename T>
void print_flist(const std::forward_list<T>& flist) {
    std::cout << "flist => ";
    for (const auto &item: flist) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}

bool compare_descending(int a, int b) {
    return a > b;
}

bool compare_ascending(int a, int b) {
    return b > a;
}
