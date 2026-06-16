#include <iostream>

class Person {

private:
    std::string name;

public:
    Person(std::string);
    std::string getName() const;
    virtual void print() const final;
};

Person::Person(std::string name): name{name} {};

std::string Person::getName() const {
    return name;
}

void Person::print() const {
    std::cout
        << "{name: "  << name
        << std::endl;
}

class Employee {

public:
    void print() const override; // <= Error 
};

int main(int argc, char const *argv[]) {
    return 0;
}
