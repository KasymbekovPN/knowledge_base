#include <iostream>
#include <map>

void _print_mmap(const std::multimap<int, std::string>&, std::string);

int main() {
    std::multimap<int, std::string> mmap {
        {1, "one"},
        {2, "two"},
        {2, "two two"},
        {3, "three"},
        {3, "three three"},
        {3, "three three three"}
    };
    _print_mmap(mmap, "BEFORE");

    mmap.erase(mmap.find(2));
    _print_mmap(mmap, "AFTER");

    return 0;
}

void _print_mmap(const std::multimap<int, std::string>& mmap, std::string label) {
    std::cout
        << "### " << label << " ###"
        << std::endl;
    for (auto &[key, value]: mmap) {
        std::cout
         << "{" << key
         << ", " << value
         << "}" << std::endl;
    }
}
