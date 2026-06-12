#include <iostream>
#include <map>

void _print_mmap(const std::multimap<int, std::string>&);

int main() {
    std::multimap<int, std::string> mmap;
    mmap.insert({
        {1, "one"},
        {3, "three"},
        {5, "five"}
    });

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
