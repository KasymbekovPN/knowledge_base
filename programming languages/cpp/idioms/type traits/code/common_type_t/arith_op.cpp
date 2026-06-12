#include <iostream>
#include <type_traits>

template<typename T, typename U>
auto add(T _t, U _u) -> std::common_type_t<T, U> {
    return _t + _u;
}

int main() {
    auto&& result = add(3.14f, 42);
    std::cout << "value: " << result << std::endl;
    std::cout << "type: " << typeid(result).name() << std::endl;
    return 0;
}
