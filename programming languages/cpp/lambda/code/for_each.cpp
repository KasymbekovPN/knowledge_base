#include <iostream>
#include <vector>
#include <algorithm>

int main() {
    const std::vector<int> v {1, 2, 3, 4, 5};
    std::for_each(v.begin(), v.end(), [](int x){
        std::cout << x * x << " ";
    });
    std::cout << std::endl;

    return 0;
}
