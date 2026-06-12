#include <iostream>
#include <map>
#include <vector>

void _print_mmap(const std::multimap<int, std::string>&);

int main() {
    std::multimap<int, std::string> mmap = {
        {1, "one"},
        {3, "three"},
        {5, "five"},
    };

    auto hint = mmap.find(3);
    if (hint != mmap.end()) {
        mmap.insert(hint, {7, "seven"});
    }

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
