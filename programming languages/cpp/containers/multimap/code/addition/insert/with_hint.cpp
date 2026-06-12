#include <iostream>
#include <map>

void _print_mmap(const std::multimap<int, std::string>&);
void _print_new(const std::multimap<int, std::string>::iterator&);

int main() {
    std::multimap<int, std::string> mmap = {
        {1, "one"},
        {3, "three"},
        {5, "five"}
    };

    auto hint = mmap.begin();
    _print_new(mmap.insert(hint, std::make_pair(7, "seven")));

    hint = mmap.begin();
    std::advance(hint, 3);
    _print_new(mmap.insert(hint, std::make_pair(2, "two")));

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
