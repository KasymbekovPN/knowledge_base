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

friend void Person::drive(const Auto&);
friend void Person::setPrice(Auto&, unsigned);

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

Auto::Auto(std::string name, unsigned price): name(name), price(price){}

void Auto::print() {
    std::cout
        << "{name: " << name
        << ", price: " << price
        << "}" << std::endl;
}

int main(int argc, char const *argv[]) {
    Auto tesla {"Tesla", 5000};
    Person tom {"Tom"};
    tesla.print();

    tom.drive(tesla);
    tom.setPrice(tesla, 5550);
    tesla.print();

    return 0;
}
