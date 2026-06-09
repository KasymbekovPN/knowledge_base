---
tags:
  - programming-language
  - cpp
  - syntax
  - oop
  - override
  - virtual
---
[[__cpp syntax oop virtual methods__|<==]]

Чтобы явным образом указать, что мы хотим переопределить функцию, а не скрыть ее, в производном классе после списка параметров функции указывается слово _override_.

```cpp
type function_name() override { /* ... */}
```

В выражении ключевое слово _override_ указывает на то, что функция будет явным образом переопределена. Если переопределение невозможно, например, из-за отличных параметров функции, то родительская функция не будет скрыта, как было бы без использования _override_, но будет ошибка компиляции..

При этом стоит отметить, что виртуальную функцию можно переопределить по всей иерархии наследования в том числе не в прямых производных классах.

```cpp
#include <iostream>

class Person {

private:
    std::string name;

public:
    Person(std::string);
    std::string getName() const;
    virtual void print() const;
    virtual void printOther() const;
};

Person::Person(std::string name): name{name} {}

std::string Person::getName() const {
    return name;
}

void Person::print() const {
    std::cout << "[Person::print] name <= " << name << std::endl;
}

void Person::printOther() const {
    std::cout << "[Person::printOther] name <= " << name << std::endl;
}

class Employee: public Person{

private:
    std::string company;

public:
    Employee(std::string, std::string);
    void print()  const override;
    // void printOther(std::string)  const override; // <= Error
};

Employee::Employee(std::string name, std::string company):
    Person{name},
    company{company} {}

void Employee::print() const{
    std::cout << "[Employee::print] name <= " << getName()
        << ", company <= " << company << std::endl;
}

// void Employee::printOther(std::string)  const {
//     std::cout << "[Employee::printOther] name <= " << getName()
//         << ", company <= " << company << std::endl;
// }

int main(int argc, char const *argv[]) {
    Person* p = new Person{"Tom"};
    p->print();
    p->printOther();

    p = new Employee{"Bob", "Company"};
    p->print();
    p->printOther();

    return 0;
}
```

```
[Person::print] name <= Tom
[Person::printOther] name <= Tom
[Employee::print] name <= Bob, company <= Company
[Person::printOther] name <= Bob
```

```
.\override_demo.cpp:37:41: error: non-virtual member function marked 'override' hides virtual member function
   37 |     void printOther(std::string)  const override; // <= Error
      |                                         ^
.\override_demo.cpp:12:18: note: hidden overloaded virtual function 'Person::printOther' declared here: different number of parameters (0 vs 1)
   12 |     virtual void printOther() const;
      |                  ^
```

---
[Виртуальные функции](https://metanit.com/cpp/tutorial/5.11.php)