#include <iostream>
#include <vector>
#include <queue>

int main() {
    std::priority_queue<int, std::vector<int>, std::greater<int>> q;
    q.push(2);
    q.push(1);
    q.push(3);
    std::cout << "Top: " << q.top() << std::endl;

    return 0;
}
