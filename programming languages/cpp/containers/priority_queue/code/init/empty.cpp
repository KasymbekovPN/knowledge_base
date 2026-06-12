#include <iostream>
#include <queue>

int main() {
    std::priority_queue<int> q;
    std::cout
        << "Is it empoty? "
        << std::boolalpha
        << q.empty()
        << std::noboolalpha
        << std::endl;

    return 0;
}
