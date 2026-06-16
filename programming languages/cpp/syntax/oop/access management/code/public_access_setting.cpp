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
        << ", age: " << age
        << "}" << std::endl;
}

class Employee: private Person {

private:
    std::string company;

public:
    Employee(std::string, unsigned, std::string);
    using Person::print;
};

Employee::Employee(std::string name, unsigned age, std::string company):
    Person(name, age),
    company(company) {}


int main(int argc, char const *argv[]) {
    Employee tom {"Tom", 42, "Company"};
    tom.print();

    return 0;
}
