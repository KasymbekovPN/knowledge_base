#include <iostream>
#include <unordered_map>

template <typename K, typename V>
void _print_map(const std::unordered_map<K, V>&);

int main() {
    std::unordered_map<std::string, int> map {
        {"one", 1},
        {"two", 2},
        {"three", 3},
        {"four", 4},
        {"five", 5}
    };
    _print_map(map);

    auto first_it = map.find("two");
    auto last_it = map.find("five");
    if (first_it != map.end() && last_it != map.end()) {
        map.erase(first_it, last_it);
    }

    _print_map(map);

    return 0;
}

template <typename K, typename V>
void _print_map(const std::unordered_map<K, V>& uomap) {
    std::cout << "###" << std::endl;
    for (auto &[key, value]: uomap) {
        std::cout
            << "{" << key
            << ", " << value
            << "}" << std::endl;
    }
}
