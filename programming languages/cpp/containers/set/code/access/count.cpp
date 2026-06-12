#include <iostream>
#include <set>

int main() {
    std::set<int> s {1, 2, 3};
    std::cout << s.count(2) << std::endl;
    std::cout << s.count(42) << std::endl;

    return 0;
}
