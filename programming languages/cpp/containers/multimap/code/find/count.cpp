#include <iostream>
#include <map>

void _test_count(const std::multimap<int, std::string>&, int);

int main() {
    std::multimap<int, std::string> mmap {
        {1, "one"},
        {2, "two"},
        {2, "two two"},
        {3, "three"},
        {3, "three three"},
        {3, "three three three"}
    };

    _test_count(mmap, 1);
    _test_count(mmap, 2);
    _test_count(mmap, 3);
    _test_count(mmap, 42);

    return 0;
}

void _test_count(const std::multimap<int, std::string>& mmap, int key) {
    std::cout
        << "Count by key " << key
        << " is " << mmap.count(key)
        << std::endl;
}
