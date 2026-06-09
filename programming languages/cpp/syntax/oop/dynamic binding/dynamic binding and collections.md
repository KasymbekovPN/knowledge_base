---
tags:
  - programming-language
  - cpp
  - syntax
  - oop
  - dynamic-binding
  - collection
---
[[__cpp syntax oop dynamic binding__|<=]]

Объекты базовых и производных классов можно хранить в одной коллекции.

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

int main(int argc, char const *argv[]) {
    Person* people[] {
        new Person{"Tom"},
        new Employee {"Bob", "Company"}
    };

    for (const auto person: people) {
        person->print();
    }

    return 0;
}
```

```
{name: Tom}
{name: Bob, company: Company}
```

---
[Особенности динамического связывания](https://metanit.com/cpp/tutorial/5.27.php)