#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

int main() {
    const std::vector<int> v0 {10, 20, 30};
    const std::vector<int> v1 {12, 18, 32};

    bool result = std::equal(
        v0.begin(),
        v0.end(),
        v1.begin(),
        [](int x, int y) {return std::abs(x - y) <= 2;});
    std::cout
        << "result <= "
        << std::boolalpha
        << result
        << std::noboolalpha
        << std::endl;

    return 0;
}
