#include <iostream>

class Person {

private:
    std::string name;

public:
    Person(std::string);
    virtual void print() const;
};

Person::Person(std::string name): name(name) {}

void Person::print() const {
    std::cout << "name <= " << name << std::endl;
}

class Employee: public Person{

private:
    std::string company;

public:
    Employee(std::string, std::string);
    void print() const;
};

Employee::Employee(std::string name, std::string company): Person{name}, company{company} {}

void Employee::print() const {
    Person::print();
    std::cout << "Works in " << company << std::endl;
}

int main(int argc, char const *argv[]) {
    Person tom {"Tom"};
    Person* p {&tom};
    p->print();

    p = new Employee{"Bob", "Company"};
    p->print();

    return 0;
}
