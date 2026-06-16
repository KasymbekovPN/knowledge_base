#include <iostream>

class Person {

protected:
    std::string name;

public:
    Person(std::string);
    virtual void printV() const;
    void print() const;
};

Person::Person(std::string name): name{name} {}

void Person::printV() const {
    std::cout << "[Person::printV] name <= " << name << std::endl;
}

void Person::print() const {
    std::cout << "[Person::print] name <= " << name << std::endl;
}

class Emplyee: public Person {

private:
    std::string company;

public:
    Emplyee(std::string, std::string);
    virtual void printV() const;
    void print() const;
};

Emplyee::Emplyee(std::string name, std::string company):
    Person{name},
    company{company} {}

void Emplyee::printV() const {
    std::cout
        << "[Emplyee::printV] name <= " << name
        << ", company <= " << company << std::endl;
}
void Emplyee::print() const {
    std::cout
        << "[Emplyee::print] name <= " << name
        << ", company <= " << company << std::endl;
}

int main(int argc, char const *argv[]) {
    Emplyee tom {"Tom", "Company"};
    tom.print();
    tom.printV();

    std::cout << std::endl << " ### Copy constructor ###" << std::endl;
    Person p0 {tom};
    p0.print();
    p0.printV();

    std::cout << std::endl << " ### Assignment ###" << std::endl;
    Person p1 = tom;
    p1.print();
    p1.printV();

    std::cout << std::endl << " ### static_cast ###" << std::endl;
    Person p2 = static_cast<Person>(tom);
    p2.print();
    p2.printV();

    return 0;
}
