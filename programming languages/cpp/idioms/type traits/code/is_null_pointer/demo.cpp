#include <iostream>
#include <type_traits>

using namespace std;

void print();

template<typename T>
void process(const T&);

int main() {
    print();

    int* p0 = new int{42};
    process(p0);
    process(nullptr);

    return 0;
}

void print() {
    cout
        << boolalpha
        << "is_null_pointer_v<decltype(nullptr)>: "
        << is_null_pointer_v<decltype(nullptr)>
        << endl
        << "is_null_pointer_v<int*>: "
        << is_null_pointer_v<int*>
        << endl
        << "is_null_pointer_v<void>: "
        << is_null_pointer_v<void>
        << endl
        << noboolalpha;
}

template<typename T>
void process(const T& _value) {
    if constexpr (is_null_pointer_v<T>){
        cout << "You passed nullptr" << endl;
    } else {
        cout << "You passed value" << endl;
    }
}
