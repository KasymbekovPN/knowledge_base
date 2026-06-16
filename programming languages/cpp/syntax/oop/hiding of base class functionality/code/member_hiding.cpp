#include <iostream>

class Integer {

protected:
    unsigned value;

public:
    Integer(unsigned);
    void printInteger() const;
};

Integer::Integer(unsigned value):
    value(value) {}

void Integer::printInteger() const {
    std::cout << "Integer::value <= " << value << std::endl;
}


class Decimal: public Integer {

protected:
    unsigned value;

public:
    Decimal(unsigned, unsigned);
    void printDecimal() const;
};

Decimal::Decimal(unsigned i, unsigned d):
    Integer(i),
    value(d) {}

void Decimal::printDecimal() const {
    std::cout << "Decimal::value <= " << value << "." << Integer::value << std::endl;
}


int main(int argc, char const *argv[]) {
    Decimal d {123, 456};
    d.printInteger();
    d.printDecimal();

    return 0;
}
