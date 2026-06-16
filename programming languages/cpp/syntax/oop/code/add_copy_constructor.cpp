#include <iostream>

class Person {

private:
    std::string name;
    unsigned age;

public:
    Person(std::string _name, unsigned _age);
    Person(const Person& other);
    void print();
};

Person::Person(std::string _name, unsigned _age): name(_name), age(_age) {}

Person::Person(const Person& other): name("Copy of " + other.name), age(other.age + 1) {}

void Person::print() {
    std::cout
        << "{name: " << name
        << ", age: " << age
        << "}" << std::endl;
}

int main(int argc, char const *argv[]) {
    Person tom {"Tom", 42};
    tom.print();

    Person tomas {tom};
    tomas.print();

    return 0;
}
