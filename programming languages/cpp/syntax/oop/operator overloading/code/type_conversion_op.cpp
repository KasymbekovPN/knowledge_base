#include <iostream>

class Counter {

private:
    int value;

public:
    Counter(int);
    operator int() const;
    operator bool() const;
};

Counter::Counter(int value): value{value} {}

Counter::operator int() const {
    return value;
}

Counter::operator bool() const {
    return value != 0;
}

void test_as_int(const Counter&, int, std::string);
void test_as_bool(const Counter&, bool, std::string);

int main(int argc, char const *argv[]) {
    Counter counter0 {-1};
    Counter counter1 {0};
    Counter counter2 {1};

    test_as_int(counter0, 0, "counter0 & 0");
    test_as_int(counter0, -1, "counter0 & -1");
    test_as_bool(counter0, true, "counter0 & true");
    test_as_bool(counter0, false, "counter0 & false");
    std::cout << std::endl;

    test_as_int(counter1, 1, "counter0 & 1");
    test_as_int(counter1, 0, "counter0 & 0");
    test_as_bool(counter1, true, "counter1 & true");
    test_as_bool(counter1, false, "counter1 & false");
    std::cout << std::endl;

    test_as_int(counter2, 2, "counter0 & 2");
    test_as_int(counter2, 1, "counter0 & 1");
    test_as_bool(counter2, true, "counter2 & true");
    test_as_bool(counter2, false, "counter2 & false");
    std::cout << std::endl;

    return 0;
}

void test_as_int(const Counter& counter, int expected, std::string message) {
    int gotten_value {static_cast<int>(counter)};
    std::cout
        << "[test_as_int] " << message << ": "
        << std::boolalpha << (gotten_value == expected)
        << std::noboolalpha << std::endl;
}

void test_as_bool(const Counter& counter, bool expected, std::string message) {
    bool gotten_value {static_cast<bool>(counter)};
    std::cout
        << "[test_as_bool] " << message << ": "
        << std::boolalpha << (gotten_value == expected)
        << std::noboolalpha << std::endl;
}
