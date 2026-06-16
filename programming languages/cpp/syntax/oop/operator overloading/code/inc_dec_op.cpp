#include <iostream>

class Counter {

private:
    int value;

public:
    Counter(int);
    void print() const;
    // prefix
    Counter& operator++();
    Counter& operator--();
    // postfix
    Counter operator++(int);
    Counter operator--(int);
};

Counter::Counter(int value): value{value} {}

void Counter::print() const {
    std::cout
        << "{value: " << value
        << "}" << std::endl;
}

Counter& Counter::operator++() {
    value++;
    return *this;
}

Counter& Counter::operator--() {
    value--;
    return *this;
}

Counter Counter::operator++(int v) {
    std::cout << "++ " << v << std::endl;
    Counter copy {*this};
    ++(*this);
    return copy;
}

Counter Counter::operator--(int v) {
    std::cout << "-- " << v << std::endl;
    Counter copy {*this};
    --(*this);
    return copy;
}

int main(int argc, char const *argv[]) {
    Counter c0 {42};
    Counter c1 = c0++;
    c1.print();

    ++c1;
    c1.print();

    return 0;
}
