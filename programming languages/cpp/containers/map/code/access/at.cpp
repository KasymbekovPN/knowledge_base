#include <iostream>
#include <map>
#include <string>

void _print_item(const std::map<std::string, int>&, std::string);

int main() {
    std::map<std::string, int> map = {
        {"one", 1},
        {"two", 2},
    };

    _print_item(map, "one");
    _print_item(map, "two");
    _print_item(map, "three");
    
    return 0;
}

void _print_item(const std::map<std::string, int>& map, std::string key) {
    try {
        int value = map.at(key);
        std::cout
            << "{key: " << key
            << ", value: " << value
            << "}" << std::endl;
    } catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
    }
}
