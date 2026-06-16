#include <iostream>

class Person {

private:
    std::string name;
    unsigned age;

public:
    Person(std::string, unsigned);
    void print() const;
};

Person::Person(std::string name, unsigned age):
    name(name),
    age(age) {}

void Person::print() const {
    std::cout
        << "{name: " << name
        << ", age: "  << age
        << "}" << std::endl;
}

class Employee: public Person{

public:
    using Person::Person;
};

int main(int argc, char const *argv[]) {
    Person tom {"Tom", 42};
    tom.print();

    Employee bob {"Bob", 43};
    bob.print();

    return 0;
}
