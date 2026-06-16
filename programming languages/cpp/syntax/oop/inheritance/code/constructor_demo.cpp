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

private:
    std::string company;

public:
    Employee(std::string, unsigned, std::string);
};

Employee::Employee(std::string name, unsigned age, std::string company):
    Person(name, age),
    company(company) {}

int main(int argc, char const *argv[]) {
    Person tom {"Tom", 42};
    tom.print();

    Employee bob {"Bob", 43, "Comp"};
    bob.print();

    return 0;
}
