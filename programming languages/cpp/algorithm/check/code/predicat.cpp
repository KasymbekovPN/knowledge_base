#include <iostream>
#include <vector>
#include <algorithm>

void _test_is_permutation(const std::vector<int>&, const std::vector<int>&);
bool _abs_equal(int, int);

int main() {
    const std::vector<int> v0 {1, 2, 3};
    const std::vector<int> v1 {2, 3, 1};
    const std::vector<int> v2 {0, 1, 2};

    _test_is_permutation(v0, v1);
    _test_is_permutation(v1, v2);

    return 0;
}

void _test_is_permutation(const std::vector<int>& v0, const std::vector<int>& v1) {
    bool result = std::is_permutation (
        v0.begin(),
        v0.end(),
        v1.begin(),
        _abs_equal
    );
    std::cout
        << "Permutation <= "
        << std::boolalpha
        << result
        << std::noboolalpha
        << std::endl;
}

bool _abs_equal(int a, int b) {
    return abs(a) == abs(b);
}
