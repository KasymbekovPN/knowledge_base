#include <iostream>
#include <type_traits>

using namespace std;

template<typename T>
void test(const T&);

int main() {
    int x {42};
    test(x);

    int* pint = &x;
    test(pint);

    std::string s {"Hello"};
    test(s);

    const char* c = "world";
    test(c);

    return 0;
}

template<typename T>
void test(const T& _value) {
    if constexpr (is_pointer_v<decay_t<T>>) {
        cout << "Pointer: " << *_value << endl;
    } else {
        cout << "Not a pointer: " << _value << endl;
    }
}
