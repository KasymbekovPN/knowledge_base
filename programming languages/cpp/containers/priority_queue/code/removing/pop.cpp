#include <iostream>
#include <queue>
#include <vector>

void _test_pop(std::priority_queue<int, std::vector<int>>&);

int main() {
    std::priority_queue<int, std::vector<int>> q;
    q.push(1);
    q.push(2);

    for (size_t i {}; i < 3; i++) {
        _test_pop(q);
    }

    return 0;
}

void _test_pop(std::priority_queue<int, std::vector<int>>& queue) {
    if (queue.size() > 0) {
        queue.pop();
        std::cout << "Size: " << queue.size();
    } else {
        std::cout << "Empty!";
    }
    std::cout << std::endl;
}
