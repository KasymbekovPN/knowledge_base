#include <iostream>

class Person {

private:
    std::string name;
    unsigned age;

public:
    Person(std::string, unsigned);
    Person(const Person&);
    void print() const;
};

Person::Person(std::string name, unsigned age):
    name(name),
    age(age) {}

Person::Person(const Person& original):
    Person(original.name, original.age) {}

void Person::print() const {
    std::cout
        << "{name: " << name
        << ", age: " << age
        << "}" << std::endl;
}

class Employee: public Person {

private:
    std::string company;

public:
    Employee(std::string, unsigned, std::string);
    Employee(const Employee&);
};

Employee::Employee(std::string name, unsigned age, std::string company):
    Person(name, age),
    company(company) {}

Employee::Employee(const Employee& original):
    Person(original),
    company(original.company) {}


int main(int argc, char const *argv[]) {
    Employee tom {"Tom", 42, "Company"};
    Employee tomas {tom};

    tomas.print();

    return 0;
}
