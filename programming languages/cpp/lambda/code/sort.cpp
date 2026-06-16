#include <iostream>
#include <vector>
#include <algorithm>

int main() {
    std::vector<int> v {9, 1, 21, 3, 42, 5};
    std::sort(v.begin(), v.end(), [](int a, int b){
        return a > b;
    });
    
    for (auto& item: v) {
        std::cout << item << " ";
    }
    std::cout << std::endl;

    return 0;
}
