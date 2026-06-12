#include <iostream>

using std::cout;
using std::endl;

unsigned long long calculate_factorial(unsigned);

int main(int argc, char const *argv[]) {
    unsigned n {5};
    auto result {calculate_factorial(n)};

    cout << n << "! => " << result << endl;

    return 0;
}

unsigned long long calculate_factorial(unsigned n) {
    if (n <= 1) {
        return 1;
    }

    return n * calculate_factorial(n - 1);
}
