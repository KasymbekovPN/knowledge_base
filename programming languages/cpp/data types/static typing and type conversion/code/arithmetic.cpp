#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    double sum {100.2};
    int hours {8};
    double perHour {sum / hours};
    cout << "per hour <= " << perHour << endl;

    int n {5};
    unsigned int x {8};
    unsigned int result {n - x};
    cout << n << " - " << x << " = result <= " << result << endl;

    return 0;
}
