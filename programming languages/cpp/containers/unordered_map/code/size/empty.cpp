#include <iostream>
#include <unordered_map>

void _test_empty(const std::unordered_map<int, int>&);

int main() {
    std::unordered_map<int, int> empty_map;
    _test_empty(empty_map);

    std::unordered_map<int, int> not_empty_map {
        {1, 1}
    };
    _test_empty(not_empty_map);

    return 0;
}

void _test_empty(const std::unordered_map<int, int>& map) {
    std::cout
        << "Is it empty? "
        << std::boolalpha
        << map.empty()
        << std::noboolalpha
        << std::endl;
}
