#include <iostream>
#include <set>

template<typename T>
void _print_set(const std::set<T>&);

int main() {
    std::set<int> set {4, 1, 3, 1, 4, 7};
    _print_set(set);

    return 0;
}

template<typename T>
void _print_set(const std::set<T>& set) {
    for (auto &item: set) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}
