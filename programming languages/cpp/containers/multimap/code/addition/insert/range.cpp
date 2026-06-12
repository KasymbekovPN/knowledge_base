#include <iostream>
#include <map>
#include <vector>

void _print_mmap(const std::multimap<int, std::string>&);

int main() {
    std::vector<std::pair<int, std::string>> source {
        {1, "one"},
        {3, "three"},
        {5, "five"}
    };
    std::multimap<int, std::string> mmap;
    mmap.insert(source.begin(), source.end());

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
