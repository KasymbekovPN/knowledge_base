#include <iostream>
#include <vector>
#include <algorithm>

class IsEven {

public:
    bool operator()(int x) const noexcept {
        return x % 2 == 0;
    }
};

int main() {
    const std::vector<int> vec {1, 2, 3, 4, 5};
    int count = std::count_if(vec.begin(), vec.end(), IsEven());
    std::cout << "Even count: " << count << std::endl;

    return 0;
}
