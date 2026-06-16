#include <iostream>

class Person {

private:
    std::string name;
    unsigned age;

public:
    Person(std::string, unsigned);
    ~Person();
    void print() const;
};

Person::Person(std::string name, unsigned age):
    name(name),
    age(age) {

    std::cout << "Person constructor" << std::endl;
}

Person::~Person() {
    std::cout << "Person distructor" << std::endl;
}

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
    ~Employee();
};

Employee::Employee(std::string name, unsigned age, std::string company):
    Person(name, age),
    company(company) {

    std::cout << "Employee constructor" << std::endl;    
}

Employee::~Employee() {
    std::cout << "Employee distructor" << std::endl;    
}


int main(int argc, char const *argv[]) {
    {
        Employee tom {"Tom", 42, "Company"};
    }
    
    return 0;
}
