#include <iostream>
#include <map>
#include <string>

void _print_pair(const std::pair<std::map<std::string, int>::iterator, bool>&);
void _print_map(const std::map<std::string, int>&);

int main() {
    std::map<std::string, int> map;
    _print_pair(map.insert(std::make_pair("hello", 1)));
    _print_pair(map.insert(std::make_pair("hello", 1)));

    // C++11
    _print_pair(map.insert({"world", 2}));
    _print_pair(map.insert({"world", 2}));

    std::map<std::string, int> map1 = {{"aaa", 3}, {"bbb", 4}};
    // C++11
    map.insert(map1.begin(), map1.end());
    _print_map(map);

    // C++17
    _print_pair(map.insert_or_assign("ccc", 42));
    _print_pair(map.insert_or_assign("ccc", 45));
    
    return 0;
}

void _print_map(const std::map<std::string, int>& m) {
    for (auto &pair: m) {
        std::cout << "{" << pair.first
            << ", " << pair.second
            << "}" << std::endl;
    }
}

void _print_pair(const std::pair<std::map<std::string, int>::iterator, bool>& pair) {
    std::cout
        << "{" << (*pair.first).first
        << ", " << (*pair.first).second
        << std::boolalpha
        << ", " << pair.second
        << std::noboolalpha
        << "}" << std::endl;
}