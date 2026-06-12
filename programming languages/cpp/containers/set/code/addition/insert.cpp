#include <iostream>
#include <set>

void _print_result(const std::pair<std::set<int>::iterator, bool>&);

int main() {
    std::set<int> set {1, 2, 3};
    _print_result(set.insert(3));
    _print_result(set.insert(4));

    return 0;
}

void _print_result(const std::pair<std::set<int>::iterator, bool>& pair) {
    std::cout
        << *pair.first
        << " <> "
        << std::boolalpha
        << pair.second
        << std::noboolalpha
        << std::endl;
}
