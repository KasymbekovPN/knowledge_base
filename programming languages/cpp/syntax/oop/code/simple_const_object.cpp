#include <iostream>

class Person {

public:
    std::string name;
    unsigned age;

    Person(std::string _name, unsigned _age);
    void print();
};

Person::Person(std::string _name, unsigned _age): name(_name), age(_age) {}

void Person::print() {
    std::cout
        << "{name: " << name
        << ", age: " << age
        << "}" << std::endl;
}

int main(int argc, char const *argv[]) {
    const Person tom {"Tome", 42};
    std::cout
        << "{name: " << tom.name
        << ", age: " << tom.age
        << "}" << std::endl;
    // tom.age++; // Error
    // tom.print(); // Error

    return 0;
}
