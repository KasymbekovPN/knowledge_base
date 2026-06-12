#include <iostream>
#include <concepts>
#include <vector>

template<typename T>
concept Container = requires(T c) {
    typename T::value_type;
    {c.begin()} -> std::input_iterator;
    {c.end()} -> std::same_as<decltype(c.begin())>;
    {c.size()} -> std::convertible_to<size_t>;
};

template<Container C>
void test(const C&);

int main() {
    test(std::vector<int>({1, 2, 3}));
    // test(42); // Error

    return 0;
}

template<Container C>
void test(const C& _container) {
    std::cout << "Size: " << _container.size() << std::endl;
}
