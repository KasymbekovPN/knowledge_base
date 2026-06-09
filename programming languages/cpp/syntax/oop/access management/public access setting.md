---
tags:
  - programming-language
  - cpp
  - syntax
  - access
---
[[__cpp syntax oop access management__|<==]]

Если к полю или методу базового класса нет доступа, но есть необходимость в использовании, то можно восстановить уровень доступа при помощи ключевого слова _using_.

```cpp
#include <iostream>

class Person {

private:
    std::string name;
    unsigned age;

public:
    Person(std::string, unsigned);
    void print() const;
};

Person::Person(std::string name, unsigned age):
    name(name),
    age(age) {}

void Person::print() const {
    std::cout
        << "{name: " << name
        << ", age: " << age
        << "}" << std::endl;
}

class Employee: private Person {

private:
    std::string company;

public:
    Employee(std::string, unsigned, std::string);
    using Person::print;
};

Employee::Employee(std::string name, unsigned age, std::string company):
    Person(name, age),
    company(company) {}

int main(int argc, char const *argv[]) {
    Employee tom {"Tom", 42, "Company"};
    tom.print();

    return 0;
}
```

```
{name: Tom, age: 42}
```

Без восстановления уровня доступа для _print_ попытка компиляции закончилась бы ошибкой.
```
.\public_access_setting.cpp:42:9: error: 'print' is a private member of 'Person'
   42 |     tom.print();
      |         ^
.\public_access_setting.cpp:25:17: note: constrained by private inheritance here
   25 | class Employee: private Person {
      |                 ^~~~~~~~~~~~~~
.\public_access_setting.cpp:18:14: note: member is declared here
   18 | void Person::print() const {
      |              ^
```

---
[Управление доступом в базовых и производных классах](https://metanit.com/cpp/tutorial/5.22.php)