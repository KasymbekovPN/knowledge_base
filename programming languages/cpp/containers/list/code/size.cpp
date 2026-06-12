#include <iostream>
#include <list>

template<typename T>
void is_empty(const std::list<T>&);

int main() {
    std::list<int> numbers0 {};
    is_empty(numbers0);

    std::list<int> numbers1 {1, 2, 1, 4, 1, 6, 1};
    is_empty(numbers1);

    return 0;
}

template<typename T>
void is_empty(const std::list<T>& list) {
    std::cout
        << "Is empty? "
        << std::boolalpha
        << list.empty()
        << std::noboolalpha
        << std::endl;
}