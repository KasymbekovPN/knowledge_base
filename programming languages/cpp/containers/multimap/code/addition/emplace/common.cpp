#include <iostream>
#include <map>
#include <vector>

void _print_mmap(const std::multimap<int, std::string>&);
void _print_new(const std::multimap<int, std::string>::iterator&);

int main() {
    std::multimap<int, std::string> mmap;
    _print_new(mmap.emplace(1, "one"));
    _print_new(mmap.emplace(1, "one one"));
    _print_new(mmap.emplace(2, "two"));

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

void _print_new(const std::multimap<int, std::string>::iterator& it) {
    std::cout
        << "NEW {kye: " << it->first
        << ", value: " << it->second
        << "}" << std::endl;
}
