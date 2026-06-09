---
tags:
  - programming-language
  - cpp
  - syntax
  - class
  - friend
---
[[__cpp syntax friend__|<==]]

Иногда целесообразно определять не отдельные дружественные функции, а определить дружественным весь класс.

```cpp
#include <iostream>

class Auto;

class Person {

private:
    std::string name;

public:
    Person(std::string);
    void drive(const Auto&);
    void setPrice(Auto&, unsigned);
};

class Auto {

friend class Person;

private:
    std::string name;
    unsigned price;

public:
    Auto(std::string, unsigned);
    void print();
};

Person::Person(std::string name): name(name) {}

void Person::drive(const Auto& car) {
    std::cout << name << " drives " << car.name << std::endl;
}

void Person::setPrice(Auto& car, unsigned price) {
    car.price = price;
}

Auto::Auto(std::string name, unsigned price): name(name), price(price) {}

void Auto::print() {
    std::cout
        << "{name: " << name
        << ", price: " << price
        << "}" << std::endl;
}

int main(int argc, char const *argv[]) {
    Person tom {"Tom"};
    Auto tesla {"Tesla", 5000};
    tesla.print();

    tom.drive(tesla);
    tom.setPrice(tesla, 8000);
    tesla.print();

    return 0;
}
```

```
{name: Tesla, price: 5000}
Tom drives Tesla
{name: Tesla, price: 8000}
```

---
[Дружественные функции и классы](https://metanit.com/cpp/tutorial/5.5.php)