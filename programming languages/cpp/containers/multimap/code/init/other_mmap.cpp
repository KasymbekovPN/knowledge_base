#include <iostream>
#include <map>

void _print_mmap(const std::multimap<int, std::string>&);

int main() {
    std::multimap<int, std::string> original_mmap {
        {1, "one"},
        {2, "two"},
        {1, "other one"}
    };

    std::multimap<int, std::string> mmap {original_mmap};
    _print_mmap(mmap);

    return 0;
}

void _print_mmap(const std::multimap<int, std::string>& mmap) {
    for (auto &[key, value]: mmap) {
        std::cout
            << "{key: " << key
            << ", value: " << value
            << "}" << std::endl;
    }
}