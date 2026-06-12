#include <iostream>
#include <set>

void _test_count_result(const std::set<int>::iterator&, const std::set<int>&);

int main() {
    std::set<int> s {1, 2, 3};
    _test_count_result(s.find(2), s);
    _test_count_result(s.find(42), s);

    return 0;
}

void _test_count_result(const std::set<int>::iterator &it, const std::set<int> &set) {
    if (it == set.end()) {
        std::cout << "Absence";
    } else {
        std::cout << *it;
    }
    std::cout << std::endl;
}