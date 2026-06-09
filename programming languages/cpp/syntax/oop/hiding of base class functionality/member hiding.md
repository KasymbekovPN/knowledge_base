---
tags:
  - programming-language
  - cpp
  - syntax
  - oop
  - class
  - hiding
---
[[__cpp syntax oop hiding__|<==]]

Производный класс может иметь переменные с тем же именем, что и базовый класс, хотя такие ситуации могут привести к путанице, и, возможно, представляют не лучший вариант наименования переменных.

```cpp
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
    std::cout
	    << "Decimal::value <= " << value
	    << "." << Integer::value << std::endl;
}

int main(int argc, char const *argv[]) {
    Decimal d {123, 456};
    d.printInteger();
    d.printDecimal();

    return 0;
}
```

```
Integer::value <= 123
Decimal::value <= 456.123
```

---
[Скрытие функционала базового класса](https://metanit.com/cpp/tutorial/5.23.php)