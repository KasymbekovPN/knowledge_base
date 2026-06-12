#include <iostream>
#include <queue>

void _test_pop(std::queue<int>&);

int main(int argc, char const *argv[]) {
    std::queue<int> q {std::deque<int>{1, 2, 3}};
    for (size_t i{}; i < 4; i++) {
        _test_pop(q);
    }

    return 0;
}

void _test_pop(std::queue<int>& q) {
    if (!q.empty()) {
        q.pop();
        std::cout << "Size: " << q.size() << std::endl;
    } else {
        std::cout << "Empty!" << std::endl;
    }
}
