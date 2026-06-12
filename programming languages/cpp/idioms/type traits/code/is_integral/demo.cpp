#include <iostream>
#include <type_traits>

using namespace std;

template<typename T>
void test(const T&);

int main(){
    test(42);
    test(new int{42});
    test(true);
    test(std::string{"hello"});

    return 0;
}

template<typename T>
void test(const T& _value) {
    if constexpr (is_integral_v<T>) {
        cout << "Integer-like: " << _value << endl;
    } else {
        cout << "Other: " << _value << endl;
    }
}
