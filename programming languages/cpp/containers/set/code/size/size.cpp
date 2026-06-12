#include <iostream>
#include <set>

void _test_empty(const std::set<int>&);

int main() {
    std::set<int> empty_set;
    _test_empty(empty_set);

    std::set<int> s1 {1};
    _test_empty(s1);

    std::set<int> s3 {1, 2, 3};
    _test_empty(s3);

    return 0;
}

void _test_empty(const std::set<int>& set) {
    std::cout << "size: " << set.size() << std::endl;
}
