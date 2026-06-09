---
tags:
  - programming-language
  - cpp
  - syntax
  - oop
  - override
---
[[__cpp syntax oop virtual methods__|<=]]

С помощью спецификатора _final_ мы можем запретить определение в производных классах функций, которые имеют то же самое имя, возвращаемый тип и список параметров, что и виртуальная функция в базовом классе. 

Также можно переопределить функцию базового класса, но запретить ее переопределение в дальнейших производных классах.

```cpp
#include <iostream>

class Person {

private:
    std::string name;

public:
    Person(std::string);
    std::string getName() const;
    virtual void print() const final;
};

Person::Person(std::string name): name{name} {};

std::string Person::getName() const {
    return name;
}

void Person::print() const {
    std::cout
        << "{name: "  << name
        << std::endl;
}

class Employee {

public:
    void print() const override; // <= Error
};

int main(int argc, char const *argv[]) {
    return 0;
}
```

```
.\overriding_prohibition.cpp:29:24: error: only virtual member functions can be marked 'override'
   29 |     void print() const override; // <= Error
      |                        ^~~~~~~~
```

---
[Запрет переопределения](https://metanit.com/cpp/tutorial/5.11.php)