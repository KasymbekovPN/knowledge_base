---
tags:
  - programming-language
  - cpp
  - syntax
  - oop
  - class
  - function
  - hiding
---
[[__cpp syntax oop hiding__|<==]]

Производный класс может определить функцию с тем же именем, что и функция в базовом классе, с тем же или другим списком параметров. Для компилятора такая функция будет существовать независимо от базового класса. И подобное определение функции в производном классе не будет переопределением функции из базового класса.

Функция _print1_ в _Employee_ скрывает функцию _print1_ класса _Person_ и при этом использует ее внутри себя.

```cpp
#include <iostream>

class Person {

public:
    Person(std::string, unsigned);
    void print0() const;
    void print1() const;

protected:
    void print(std::string) const;

private:
    std::string name;
    unsigned age;
};

Person::Person(std::string name, unsigned age):
    name(name),
    age(age) {}

void Person::print0() const {
    print("Person::print0");
}

void Person::print1() const {
    print("Person::print1");
}

void Person::print(std::string label) const {
    std::cout
        << "[" << label << "] "
        << "{name: " << name
        << ", age: " << age
        << "}" << std::endl;
}

class Employee: public Person {

private:
    std::string company;

public:
    Employee(std::string, unsigned, std::string);
    void print0() const;
};

Employee::Employee(std::string name, unsigned age, std::string company):
    Person(name, age),
    company(company) {}

void Employee::print0() const {
    Person::print0();
    std::cout
        << "[Employee] {company: "
        << company << "}" << std::endl;
}

int main(int argc, char const *argv[]) {
    Employee tom {"Tom", 42, "Company"};
    tom.print0();
    tom.print1();

    return 0;
}
```

```
[Person::print0] {name: Tom, age: 42}
[Employee] {company: Company}
[Person::print1] {name: Tom, age: 42}
```

---
[Скрытие функционала базового класса](https://metanit.com/cpp/tutorial/5.23.php)