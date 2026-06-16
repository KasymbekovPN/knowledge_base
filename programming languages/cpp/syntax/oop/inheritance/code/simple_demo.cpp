#include <iostream>

class Person {

public:
    std::string name;
    unsigned age;

    void print() const;
};

void Person::print() const {
    std::cout
        << "{name: " << name
        << ", age: " << age
        << "}" << std::endl;
}

class Employee: public Person {

public:
    std::string company;
};

int main(int argc, char const *argv[]) {
    Person tom;
    tom.name = "Tom";
    tom.age = 42;
    tom.print();

    Employee bob;
    bob.name = "Bob";
    bob.age = 43;
    bob.company = "Some company";
    bob.print();

    return 0;
}
