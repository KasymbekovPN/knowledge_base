#include <iostream>
#include <map>

void _test_count(const std::map<std::string, int>&, std::string);

int main() {
    std::map<std::string, int> map = {
        {"one", 1},
        {"two", 2},
    };
    _test_count(map, "one");
    _test_count(map, "two");
    _test_count(map, "three");

    return 0;
}

void _test_count(const std::map<std::string, int>& map, std::string key) {
    std::cout
        << "{key: " << key
        << ", count: " << map.count(key)
        << "}" << std::endl;
}
