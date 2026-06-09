---
tags:
  - programming-language
  - cpp
  - syntax
  - conversion
  - smart-pointer
---
[[__cpp syntax oop type conversion__|<=]]

smart-указатели на базовый класс также могут указывать на объект производного класса.

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
    std::unique_ptr<Person> bob {
	    std::make_unique<Employee>("Bob", "Google")
	};
    bob->print();

    std::shared_ptr<Employee> tom {
	    std::make_shared<Employee>("Tom", "Microsoft")
	};
    tom->print();

    std::shared_ptr<Employee> sam {
	    std::make_unique<Employee>("Sam", "Jetbrains")
	};
    std::shared_ptr<Person> person {sam};
    person->print();

    return 0;
}
```

```
{name: Bob, company: Google}
{name: Tom, company: Microsoft}
{name: Sam, company: Jetbrains}
```

---
[Преобразование типов](https://metanit.com/cpp/tutorial/5.25.php)