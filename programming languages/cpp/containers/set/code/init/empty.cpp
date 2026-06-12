#include <iostream>
#include <set>

int main() {
    std::set<int> empty_set;
    std::cout
        << "Is it empty? "
        << std::boolalpha
        << empty_set.empty()
        << std::noboolalpha
        << std::endl;

    return 0;
}
