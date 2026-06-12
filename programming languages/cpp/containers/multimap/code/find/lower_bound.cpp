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

    auto it = mmap.lower_bound(2);
    std::cout
        << "{" << it->first
        << ", " << it->second
        << "}" << std::endl;

    return 0;
}
