#include <iostream>
#include <set>

int main() {
    std::set<int> s {1, 2, 3};
    std::cout << "size: " << s.size() << std::endl;

    s.clear();
    std::cout << "size: " << s.size() << std::endl;

    return 0;
}
