#include <iostream>

class Counter {

private:
    int value;

public:
    Counter(int);
    void print() const;
    Counter operator - () const;
};

Counter::Counter(int value): value{value} {}

void Counter::print() const {
    std::cout
        << "{value: " << value
        << "}" << std::endl;
}

Counter Counter::operator - () const {
    return Counter{-value};
}

int main(int argc, char const *argv[]) {
    Counter c0{42};
    c0.print();

    Counter c1 = -c0;
    c1.print();
    
    return 0;
}
