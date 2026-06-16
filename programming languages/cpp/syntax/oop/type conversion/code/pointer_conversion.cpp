#include <iostream>

class Person {

private:
    std::string name;

public:
    Person(std::string);
    std::string getName() const;
    virtual void print() const;
};

Person::Person(std::string name): name{name} {}

std::string Person::getName() const {
    return name;
}

void Person::print() const {
    std::cout
        << "{name: " << getName()
        << "}" << std::endl;
}


class Employee: public Person {

private:
    std::string company;

public:
    Employee(std::string, std::string);
    std::string getCompany() const;
    void print() const override;
};

Employee::Employee(std::string name, std::string company):
    Person{name},
    company{company} {}

std::string Employee::getCompany() const {
    return company;
}

void Employee::print() const {
    std::cout
        << "{name: " << getName()
        << ", company: " << getCompany()
        << "}" << std::endl;
}

int main(int argc, char const *argv[]) {
    std::cout << "### Converting to pointer to base type ###" << std::endl;
    Person* p0 {new Employee{"Bob A.", "Company"}};
    p0->print();

    Employee sam {"Sam", "Company"};
    std::cout << "### Ref. of base type points to derived object ###" << std::endl;
    Person& p1 {sam};
    p1.print();

    std::cout << "### Conversion of ref. of derived type to ref. of base type ###" << std::endl;
    Employee& employee{sam};
    Person& p2 {employee};
    p2.print();

    return 0;
}
