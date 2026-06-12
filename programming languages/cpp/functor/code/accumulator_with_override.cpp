#include <iostream>

using std::cout;
using std::endl;

class Accumulator {

private:
    static constexpr double DEFAULT_VALUE {0.0};
    double value;

public:
    Accumulator() noexcept: value{DEFAULT_VALUE} {}

    double operator()(const int x) noexcept {
        return value += x;
    }

    double operator()(const double x) noexcept {
        return value += x;
    }

    double get() const noexcept {
        return value;
    }

    double reset() noexcept {
        double copied_value = value;
        value = DEFAULT_VALUE;

        return copied_value;
    }
};

int main() {
    Accumulator acc;
    cout << acc.get() << endl;
    cout << acc(12.3) << endl;
    cout << acc(42) << endl;
    cout << acc.reset() << endl;
    cout << acc.get() << endl;

    return 0;
}
