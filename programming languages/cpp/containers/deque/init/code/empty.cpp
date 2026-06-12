#include <iostream>
#include <deque>

int main() {
    std::deque<int> deq;
    std::cout
        << "Is empty? "
        << std::boolalpha
        << deq.empty()
        << std::noboolalpha
        << std::endl;

    return 0;
}
