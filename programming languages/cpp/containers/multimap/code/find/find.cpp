#include <iostream>
#include <map>

void _test_find(const std::multimap<int, std::string>&, int);

int main() {
    std::multimap<int, std::string> mmap {
        {1, "one"},
        {1, "one one"},
        {2, "two"},
        {2, "two two"},
        {3, "three"},
        {3, "three three"}
    };

    _test_find(mmap, 1);
    _test_find(mmap, 2);
    _test_find(mmap, 3);
    _test_find(mmap, 42);

    return 0;
}

void _test_find(const std::multimap<int, std::string>& mmap, int key) {
    auto it = mmap.find(key);
    if (it != mmap.end()) {
        std::cout
            << "{key: " << it->first
            << ", value: "  << it->second
            << "}" << std::endl;
    } else {
        std::cout << "Not found" << std::endl;
    }
}