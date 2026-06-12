#include <iostream>
#include <map>

void _test_empty(const std::map<int, int>&);

int main() {
    const std::map<int, int> map {{1, 100}, {2, 42}};
    _test_empty(map);

    const std::map<int, int> empty_map {};
    _test_empty(empty_map);

    return 0;
}

void _test_empty(const std::map<int, int>& map) {
    std::cout
        << (map.empty() ? "It's empty" : "It isn't empty")
        << std::endl;
}
