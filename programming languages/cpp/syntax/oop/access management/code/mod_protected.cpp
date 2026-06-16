#include <iostream>

class Person {

private:
    unsigned age;

protected:
    std::string name;

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

class Employee: public Person {

private:
    std::string company;

public:
    Employee(std::string, unsigned, std::string);
    void printEmployee() const;
    // void printEmployeeFull() const;
};

Employee::Employee(std::string name, unsigned age, std::string company): Person(name, age), company(company) {}

void Employee::printEmployee() const {
    std::cout
        << "Worker '" << name
        << "' works in '" << company << "'" << std::endl;
}

// void Employee::printEmployeeFull() const {
//     std::cout
//         << "{name: " << name
//         << ", age: " << age // <= Error
//         << "}" << std::endl;
// }

int main(int argc, char const *argv[]) {
    Employee tom {"Tom", 42, "Company"};
    tom.printEmployee();

    return 0;
}
