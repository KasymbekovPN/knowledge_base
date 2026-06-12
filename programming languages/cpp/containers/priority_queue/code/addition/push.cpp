#include <iostream>
#include <queue>

int main() {
    std::priority_queue<int> q;
    q.push(1);
    q.push(2);
    q.push(3);
    std::cout << "size: " << q.size() <<  std::endl;

    return 0;
}
