---
tags:
  - programming-language
  - cpp
  - syntax
  - access
  - protected
---
[[__cpp syntax oop access management__|<==]]

Если переменные или функции в базовом классе являются закрытыми, то есть объявлены со спецификатором _private_ то, производный класс хотя и наследует эти переменные и функции, но не может к ним обращаться.

```cpp
#include <iostream>

class Person {

private:
    unsigned age;

protected:
    std::string name;

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

class Employee: public Person {

private:
    std::string company;

public:
    Employee(std::string, unsigned, std::string);
    void printEmployee() const;
    // void printEmployeeFull() const;
};

Employee::Employee(std::string name, unsigned age, std::string company): 
	Person(name, age),
	company(company) {}

void Employee::printEmployee() const {
    std::cout
        << "Worker '" << name
        << "' works in '" << company << "'" << std::endl;
}

// void Employee::printEmployeeFull() const {
//     std::cout
//         << "{name: " << name
//         << ", age: " << age // <= Error
//         << "}" << std::endl;
// }


int main(int argc, char const *argv[]) {
    Employee tom {"Tom", 42, "Company"};
    tom.printEmployee();

    return 0;
}
```

```
Worker 'Tom' works in 'Company'
```

Если раскоментировать метод  printEmployeeFull:
```
.\mod_protected.cpp:49:25: error: 'age' is a private member of 'Person'
   49 |         << ", age: " << age // <= Error
      |                         ^
.\mod_protected.cpp:6:14: note: declared private here
    6 |     unsigned age;
      |              ^
```


---
[Управление доступом в базовых и производных классах](https://metanit.com/cpp/tutorial/5.22.php)