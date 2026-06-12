#include <iostream>
#include <unordered_map>

void _test_size(const std::unordered_map<int, int>&);

int main() {
    std::unordered_map<int, int> empty_map;
    _test_size(empty_map);

    std::unordered_map<int, int> not_empty_map {
        {1, 1}
    };
    _test_size(not_empty_map);

    return 0;
}

void _test_size(const std::unordered_map<int, int>& map) {
    std::cout
        << "Size: "
        << map.size()
        << std::endl;
}
