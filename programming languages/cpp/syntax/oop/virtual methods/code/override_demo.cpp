#include <iostream>

class Person {

private:
    std::string name;

public:
    Person(std::string);
    std::string getName() const;
    virtual void print() const;
    virtual void printOther() const;
};

Person::Person(std::string name): name{name} {}

std::string Person::getName() const {
    return name;
}

void Person::print() const {
    std::cout << "[Person::print] name <= " << name << std::endl;
}

void Person::printOther() const {
    std::cout << "[Person::printOther] name <= " << name << std::endl;
}

class Employee: public Person{

private:
    std::string company;

public:
    Employee(std::string, std::string);
    void print()  const override;
    // void printOther(std::string)  const override; // <= Error
};

Employee::Employee(std::string name, std::string company):
    Person{name},
    company{company} {}

void Employee::print() const{
    std::cout << "[Employee::print] name <= " << getName()
        << ", company <= " << company << std::endl;
}

// void Employee::printOther(std::string)  const {
//     std::cout << "[Employee::printOther] name <= " << getName()
//         << ", company <= " << company << std::endl;
// }

int main(int argc, char const *argv[]) {
    Person* p = new Person{"Tom"};
    p->print();
    p->printOther();

    p = new Employee{"Bob", "Company"};
    p->print();
    p->printOther();

    return 0;
}
