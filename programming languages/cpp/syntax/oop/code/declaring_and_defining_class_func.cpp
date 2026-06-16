#include <iostream>

class Person {

private:
    std::string name;
    unsigned age;

public:
    Person(std::string _name, unsigned _age);
    Person(std::string _name);
    void print();
};

Person::Person(std::string _name, unsigned _age): name(_name), age(_age) {}

Person::Person(std::string _name): Person(_name, 42) {}

void Person::print() {
    std::cout
        << "{name: " << name
        << ", age: " << age
        << "}" << std::endl;
}

int main(int argc, char const *argv[]) {
    Person tom {"Tom", 24};
    tom.print();

    Person bob {"Bob"};
    bob.print();

    return 0;
}
