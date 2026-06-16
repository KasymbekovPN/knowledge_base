#include <iostream>

class Person {

public:
    std::string name;
    mutable unsigned age;

    Person(std::string _name, unsigned _age);
    void print() const;
};

Person::Person(std::string _name, unsigned _age): name(_name), age(_age) {}

void Person::print() const{
    std::cout
        << "{name: " << name
        << ", age: " << age
        << "}" << std::endl;
}

int main(int argc, char const *argv[]) {
    const Person tom {"Tome", 42};
    tom.age++;
    tom.print();

    return 0;
}
