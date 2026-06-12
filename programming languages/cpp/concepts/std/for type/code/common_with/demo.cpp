#include <iostream>
#include <concepts>

template<typename T, typename U>
requires std::common_with<T, U>
void test(const T&, const U&);

int main(){
    test(42, 3.14159);
    test("hi", std::string("hello"));

    return 0;
}

template<typename T, typename U>
requires std::common_with<T, U>
void test(const T& _t, const U& _u) {
    std::cout
        << "Both can be used as type: "
        << typeid(
            std::common_type_t<T, U>
        ).name()
        << std::endl;
}
