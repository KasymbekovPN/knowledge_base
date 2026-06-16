#include <iostream>
#include <memory>

class Person {

private:
    std::string name;
public:
    Person(std::string);
    virtual ~Person();
};

Person::Person(std::string name): name{name} {}

Person::~Person() {
    std::cout << "Person deleted" << std::endl;
}

class Employee: public Person {

private:
    std::string company;
public:
    Employee(std::string, std::string);
    ~Employee();
};

Employee::Employee(std::string name, std::string company):
    Person{name}, company{company} {}

Employee::~Employee() {
    std::cout << "Employee deleted" << std::endl;
}

int main(int argc, char const *argv[]) {
    std::unique_ptr<Person> sam {std::make_unique<Employee>("Sam", "Company")};
    return 0;
}
