#include <iostream>
#include <vector>
#include <span>

int main(int argc, char const *argv[]) {
    std::vector<int> vec {1, 2, 3};
    std::span<int> s0 {vec};

    for (size_t i {}; i < s0.size(); i++) {
        std::cout << s0[i] << std::endl;
    }
    

    return 0;
}
