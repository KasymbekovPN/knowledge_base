#include <iostream>
#include <vector>
#include <algorithm>

void _test_equal(const std::vector<int>&, const std::vector<int>&);

int main() {
    std::vector<int> v0 = {1, 2, 3, 4};
    std::vector<int> v1 = {1, 2, 3, 4};
    std::vector<int> v2 = {1, 2, 3, 5};
    _test_equal(v0, v1);
    _test_equal(v0, v2);

    return 0;
}

void _test_equal(const std::vector<int>& v0, const std::vector<int>& v1) {
    if (std::equal(v0.begin(), v0.end(), v1.begin())) {
        std::cout << "Vectorts are equal" << std::endl;
    } else {
        std::cout << "Vectorts are not equal" << std::endl;
    }
}
