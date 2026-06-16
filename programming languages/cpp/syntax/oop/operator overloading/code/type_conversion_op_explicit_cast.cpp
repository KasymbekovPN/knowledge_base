#include <iostream>

class Counter {

private:
    int value;

public:
    Counter(int);
    explicit operator int() const;
};

Counter::Counter(int value): value{value} {}

Counter::operator int() const {
    return value;
}

int main(int argc, char const *argv[]) {
    Counter counter {42};

    int i0 = static_cast<int>(counter);
    // int i1 = counter; // <= Error

    return 0;
}
