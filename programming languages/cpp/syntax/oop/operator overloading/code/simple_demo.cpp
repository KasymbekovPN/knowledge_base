#include <iostream>

class Counter {
private:
    int value;

public:
    Counter(int);
    int getValue() const;
    void print() const;
    Counter operator + (const Counter&) const;
    int operator + (int) const;
};

Counter::Counter(int value): value(value) {}

int Counter::getValue() const {
    return value;
}

void Counter::print() const {
    std::cout
        << "{value: " << getValue()
        << "}" << std::endl;
}

Counter Counter::operator + (const Counter& instance) const {
    return Counter {value + instance.getValue()};
}

int Counter::operator + (int other) const {
    return getValue() + other;
}

Counter operator * (const Counter&, const Counter&);

int main(int argc, char const *argv[]) {
    auto c0 = Counter{11};
    auto c1 = Counter{12};
    auto c2 = Counter{13};

    auto r0 = c0 + c1;
    r0.print();

    auto r1 = c2 + 100;
    std::cout << "value <= " << r1 << std::endl;

    auto r2 = c1 * c2;
    r2.print();

    return 0;
}

Counter operator * (const Counter& c0, const Counter& c1) {
    return Counter {c0.getValue() * c1.getValue()};
}
