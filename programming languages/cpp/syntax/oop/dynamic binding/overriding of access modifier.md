---
tags:
  - programming-language
  - cpp
  - syntax
  - override
  - virtual
---
[[__cpp syntax oop dynamic binding__|<=]]

Виртуальные функции позволяют нам обойти ограничения на доступ к функциям.

```cpp
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
    void print() const override;

public:
    Employee(std::string, std::string);
    std::string getCompany() const;
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
    Employee bob {"Bob", "Company"};
    // bob.print(); // <= Error

    Person* person {&bob};
    person->print();

    return 0;
}
```

```
{name: Bob, company: Company}
```

```
.\override_access_mod.cpp:52:9: error: 'print' is a private member of 'Employee'
   52 |     bob.print(); // <= Error
      |         ^
.\override_access_mod.cpp:29:10: note: declared private here
   29 |     void print() const override;
      |          ^
```

---
[Особенности динамического связывания](https://metanit.com/cpp/tutorial/5.27.php)