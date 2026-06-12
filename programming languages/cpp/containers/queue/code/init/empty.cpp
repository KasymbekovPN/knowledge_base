#include <iostream>
#include <queue>

int main() {
    std::queue<int> q;
    std::cout
        << "Queue is empty? "
        << std::boolalpha
        << q.empty()
        << std::noboolalpha
        << std::endl;

    return 0;
}
