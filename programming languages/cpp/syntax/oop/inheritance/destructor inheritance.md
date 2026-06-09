---
tags:
  - programming-language
  - cpp
  - syntax
  - oop
  - class
  - inheritance
  - distructor
---
[[__cpp syntax oop inheritance__|<==]]

Уничтожение объекта производного класса может вовлекать как собственно деструктор производного класса, так и деструктор базового класса.

По консольному выводу мы видим, что при создании объекта _Employee_ сначала вызывается конструктор базового класса _Person_ и затем собственно конструктор _Employee_. А при удалении объекта _Employee_ процесс идет в обратном порядке - сначала вызывается деструктор производного класса и затем деструктор базового класса. 

```cpp
#include <iostream>

class Person {

private:
    std::string name;
    unsigned age;

public:
    Person(std::string, unsigned);
    ~Person();
    void print() const;
};

Person::Person(std::string name, unsigned age):
    name(name),
    age(age) {

    std::cout << "Person constructor" << std::endl;
}

Person::~Person() {
    std::cout << "Person distructor" << std::endl;
}
 
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
    ~Employee();
};

Employee::Employee(std::string name, unsigned age, std::string company):
    Person(name, age),
    company(company) {
    
    std::cout << "Employee constructor" << std::endl;    
}

Employee::~Employee() {
    std::cout << "Employee distructor" << std::endl;    
}


int main(int argc, char const *argv[]) {
    {
        Employee tom {"Tom", 42, "Company"};
    }
    return 0;
}
```

```
Person constructor
Employee constructor
Employee distructor
Person distructor
```

---
[Наследование](https://metanit.com/cpp/tutorial/5.10.php)