#include <iostream>
#include <unordered_map>

void _print_map(const std::unordered_map<int, std::string>&);

int main() {
    std::unordered_map<int, std::string> map {
        {1, "one"},
        {2, "two"},
        {3, "three"}
    };
    _print_map(map);

    return 0;
}

void _print_map(const std::unordered_map<int, std::string>& uomap) {
    for (auto &[key, value]: uomap) {
        std::cout
            << "{ " << key
            << ", " << value
            << " }" << std::endl;
    }
}
