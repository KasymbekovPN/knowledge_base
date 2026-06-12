#include <iostream>
#include <map>

int main() {
    std::multimap<int, std::string> mmap {
        {1, "one"},
        {1, "one one"},
        {2, "two"},
        {2, "two two"},
        {3, "three"},
        {3, "three three"}
    };

    auto range = mmap.equal_range(2);
    for (auto it {range.first}; it != range.second; it++) {
        std::cout
            << "{key: " << it->first
            << ", value: " << it->second
            << "}" << std::endl;
    }

    return 0;
}
