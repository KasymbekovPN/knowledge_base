#include <iostream>
#include <unordered_map>

int main() {
    std::unordered_map<int, int> map {
        {1, 1},
        {2, 2},
        {3, 3}
    };
    std::cout << "LF: " << map.load_factor() << std::endl;

    // map.max_load_factor(0.5);
    map.emplace(4, 4);
    std::cout << "LF: " << map.load_factor() << std::endl;
 
    return 0;
}
