#include <iostream>
#include <map>

void _print_map(const std::map<std::string, int>&);

int main() {
    std::map<std::string, int> map {
        {"1 one", 1},
        {"2 two", 2},
        {"3 three", 3},
        {"4 four", 4},
        {"5 five", 5}
    };
    _print_map(map);

    map.erase("1 one"); // by key

    map.erase(map.find("2 two")); // by iterator

    auto it = map.begin();
    std::advance(it, 1);
    map.erase(it, map.end()); // by range
    _print_map(map);

    return 0;
}

void _print_map(const std::map<std::string, int>& map) {
    std::cout << " ##### " << std::endl;
    for (auto &entry: map) {
        std::cout
            << "{key: " << entry.first
            << ", value: " << entry.second
            << "}" << std::endl;
    }
}