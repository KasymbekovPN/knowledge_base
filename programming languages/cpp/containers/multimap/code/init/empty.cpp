#include <iostream>
#include <map>

int main() {
    std::multimap<int, int> empty_map;
    std::cout
        << "map size: "
        << empty_map.size() 
        << std::endl;

    return 0;
}
