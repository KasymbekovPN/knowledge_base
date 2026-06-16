#include <iostream>
#include <utility>

using std::cout;
using std::endl;

void print(int& x) {
    cout << "lvalue: " << x << endl;
}

void print(int&& x) {
    cout << "rvalue: " << x << endl;
}

template<typename T>
void wrapper(T&& x) {
    print(std::forward<T>(x)); // perfect foirwarding
}

int main() {
    int a {42};
    wrapper(a);
    wrapper(12);
    wrapper(std::move(a));

    return 0;
}
