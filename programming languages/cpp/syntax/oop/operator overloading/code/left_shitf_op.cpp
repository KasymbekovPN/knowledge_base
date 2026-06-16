#include <iostream>

class Counter {

private:
    int value;

public:
    Counter(int);
    int getValue() const;
};

Counter::Counter(int value): value{value} {}

int Counter::getValue() const {
    return value;
}

std::ostream& operator<<(std::ostream&, const Counter&);

int main(int argc, char const *argv[]) {
    Counter c0 {42};
    Counter c1 {43};

    std::cout << c0 << std::endl;
    std::cout << c1 << std::endl;

    return 0;
}

std::ostream& operator<<(std::ostream& stream, const Counter& counter) {
    stream << "{value: " << counter.getValue() << "}";
    return stream;
}
