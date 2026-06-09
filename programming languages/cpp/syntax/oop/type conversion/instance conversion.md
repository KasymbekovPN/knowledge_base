---
tags:
  - programming-language
  - cpp
  - syntax
  - oop
  - conversion
---
[[__cpp syntax oop type conversion__|<==]]

Объект производного класса одновременно является объектом базового класса. Поэтому преобразования из производного типа в базовый выполняются автоматически.

Здесь класс _Person_ является базовым, а _Employee_ производным. Поэтому компилятор может автоматически преобразовать объект _Employee_ в тип _Person_. 

Это можно сделать:
- через конструктор копирования
- через операцию присваивания
- через _static_cast_

```cpp
#include <iostream>

class Person {

protected:
    std::string name;

public:
    Person(std::string);
    virtual void printV() const;
    void print() const;
};

Person::Person(std::string name): name{name} {}

void Person::printV() const {
    std::cout << "[Person::printV] name <= " << name << std::endl;
}

void Person::print() const {
    std::cout << "[Person::print] name <= " << name << std::endl;
}

class Emplyee: public Person {

private:
    std::string company;

public:
    Emplyee(std::string, std::string);
    virtual void printV() const;
    void print() const;
};

Emplyee::Emplyee(std::string name, std::string company):
    Person{name},
    company{company} {}

void Emplyee::printV() const {
    std::cout
        << "[Emplyee::printV] name <= " << name
        << ", company <= " << company << std::endl;
}

void Emplyee::print() const {
    std::cout
        << "[Emplyee::print] name <= " << name
        << ", company <= " << company << std::endl;
}

int main(int argc, char const *argv[]) {
    Emplyee tom {"Tom", "Company"};
    tom.print();
    tom.printV();

    std::cout << std::endl << " ### Copy constructor ###" << std::endl;
    Person p0 {tom};
    p0.print();
    p0.printV();

    std::cout << std::endl << " ### Assignment ###" << std::endl;
    Person p1 = tom;
    p1.print();
    p1.printV();

    std::cout << std::endl << " ### static_cast ###" << std::endl;
    Person p2 = static_cast<Person>(tom);
    p2.print();
    p2.printV();

    return 0;
}
```

```
[Emplyee::print] name <= Tom, company <= Company
[Emplyee::printV] name <= Tom, company <= Company

 ### Copy constructor ###
[Person::print] name <= Tom
[Person::printV] name <= Tom

 ### Assignment ###
[Person::print] name <= Tom
[Person::printV] name <= Tom

 ### static_cast ###
[Person::print] name <= Tom
[Person::printV] name <= Tom
```

---
[Преобразование типов](https://metanit.com/cpp/tutorial/5.25.php)