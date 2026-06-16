#include <iostream>

class Counter {

private:
    int value;
public:
    Counter(int);
    void print() const;
    bool operator == (const Counter& counter) const;
    bool operator != (const Counter& counter) const;
    bool operator > (const Counter& counter) const;
    bool operator < (const Counter& counter) const;
};

Counter::Counter(int value): value{value} {}

void Counter::print() const {
    std::cout
        << "{value: " << value
        << "}" << std::endl;
}

bool Counter::operator == (const Counter& counter) const {
    return value == counter.value;
}

bool Counter::operator != (const Counter& counter) const {
    return value != counter.value;
}

bool Counter::operator > (const Counter& counter) const {
    return value > counter.value;
}

bool Counter::operator < (const Counter& counter) const {
    return value < counter.value;
}

void print(std::string, bool);

int main(int argc, char const *argv[]) {
    Counter c0 {12};
    Counter c1 {42};

    c0.print();
    c1.print();

    print("==", c0 == c1);
    print("!=", c0 != c1);
    print("<", c0 < c1);
    print(">", c0 > c1);

    return 0;
}

void print(std::string op, bool expr) {
    std::cout
        << "c0 " << op << " c1 => "
        << std::boolalpha
        << expr
        << std::noboolalpha
        << std::endl;
}
