#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    double sum {100.2};
    unsigned int hours {8};
    unsigned int perHour {static_cast<unsigned int>(sum / hours)};
    unsigned int perHour1 {(unsigned int) sum / hours};
    cout << "static_cast<unsigned int>(" << sum << " / " << hours << ") => " << perHour << endl;
    cout << "(unsigned int) " << sum << " / "  << hours << " => " << perHour1 << endl;

    return 0;
}
