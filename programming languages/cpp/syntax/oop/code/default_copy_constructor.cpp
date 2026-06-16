#include <iostream>

class Person {

private:
    std::string name;
    unsigned age;

public:
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
    Person tom {"Tom", 42};
    Person tomas {tom};
    tom.print();
    tomas.print();

    return 0;
}
