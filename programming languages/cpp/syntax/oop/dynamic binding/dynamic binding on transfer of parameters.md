---
tags:
  - programming-language
  - cpp
  - syntax
  - oop
  - dynamic-binding
  - parameter-transfer
---
[[__cpp syntax oop dynamic binding__|<=]]

Если необходимо обеспечить динамическое связывание при передаче параметров в функцию, то такой параметр должен представлять ссылку или указатель на объект базового типа.

В данном случае функция `printPerson` в качестве параметра принимает константную ссылку на объект типа _Person_, коим в реальности также может быть объект _Employee_. Поэтому при вызове функции _print_ программа будет динамически решать, какую именно реализацию функции вызвать.

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

void printPerson(const Person*);

int main(int argc, char const *argv[]) {
    Person* tom {new Person{"Tom"}};
    printPerson(tom);

    Employee* bob {new Employee{"Bob", "Company"}};
    printPerson(bob);
  
    return 0;
}

void printPerson(const Person* person) {
    if (person) {
        person->print();
    }
}
```

```
{name: Tom}
{name: Bob, company: Company}
```

---
[Особенности динамического связывания](https://metanit.com/cpp/tutorial/5.27.php)