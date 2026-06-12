#include <iostream>
#include <vector>
#include <span>

template <typename T>
void _print_span(const std::span<T>&);

int main(int argc, char const *argv[]) {
    std::vector<int> vec {1, 2, 3, 4, 5, 7};

    std::span<int> s0 {vec};
    _print_span(s0);

    std::span<int> s1 {vec.begin(), vec.begin() + 3};
    _print_span(s1);

    return 0;
}

template <typename T>
void _print_span(const std::span<T>& s) {
    std::cout << "{";
    std::string delimiter = "";
    for (int &item: s) {
        std::cout << delimiter << item;
        delimiter = ", ";
    }
    std::cout << "}" << std::endl;
}
