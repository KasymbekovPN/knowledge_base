#include <iostream>
#include <map>

int main() {
    const std::map<int, int> map {
        {1, 100},
        {2, 42}
    };
    std::cout << "Size: " << map.size() << std::endl;

    return 0;
}
