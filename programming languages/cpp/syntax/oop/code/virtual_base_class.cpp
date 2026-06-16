#include <iostream>

class Person {

private:
    std::string name;

public:
    Person(std::string name);
    ~Person();
    void print() const;
};

Person::Person(std::string name): name{name} {
    std::cout << "Person '" << name << "' created" << std::endl;
}

Person::~Person() {
    std::cout << "Person '" << name << "' deleted" << std::endl;
}

void Person::print() const {
    std::cout << "{name: " << name << "}" << std::endl;
}

class Student: public Person {

public:
    Student(std::string name): Person{name} {}
};

class Employee: public Person {

public:
    Employee(std::string name): Person{name} {}
};

class StudentEmployee: public Student, public Employee {

public:
    StudentEmployee(std::string name): Student{name}, Employee{name} {}
};

class StudentV: public virtual Person {

public:
    StudentV(std::string name): Person{name} {}
};

class EmployeeV: public virtual Person {

public:
    EmployeeV(std::string name): Person{name} {}
};

class StudentEmployeeV: public StudentV, public EmployeeV {

public:
    StudentEmployeeV(std::string name): Person{name}, StudentV{name}, EmployeeV{name} {}
};

int main(int argc, char const *argv[]) {
    std::cout << "### First part ###" << std::endl;
    {
        StudentEmployee se{"Bob"};
        // se.print(); // <= Error
    }

    std::cout << "### Second part ###" << std::endl;
    {
        StudentEmployeeV sev{"Tom"};
        sev.print();
    }

    return 0;
}
