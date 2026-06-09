---
tags:
  - programming-language
  - cpp
  - syntax
  - oop
  - pointer
  - conversion
---
[[__cpp syntax oop type conversion__|<=]]

- Указатель на объект производного класса можно преобразовать автоматически в указатель на объект базового типа.
- Подобным образом можно создать указатель производного класса и преобразовать автоматически в указатель на базовый тип.
- То же самое касается ссылок.

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
    std::cout << "### Converting to pointer to base type ###" << std::endl;
    Person* p0 {new Employee{"Bob A.", "Company"}};
    p0->print();

    Employee sam {"Sam", "Company"};
    std::cout << "### Ref. of base type points to derived object ###" << std::endl;
    Person& p1 {sam};
    p1.print();

    std::cout << "### Conversion of ref. of derived type to ref. of base type ###" << std::endl;
    Employee& employee{sam};
    Person& p2 {employee};
    p2.print();
    
    return 0;
}
```

```
### Converting to pointer to base type ###
{name: Bob A., company: Company}
### Ref. of base type points to derived object ###
{name: Sam, company: Company}
### Conversion of ref. of derived type to ref. of base type ###
{name: Sam, company: Company}
```

В некоторых случаях возможно приведение в обратную сторону - от базового к производному.

- автоматически оно не выполняется, нужно использовать преобразования, например, _statis_cast_
- работоспособность зависит от типа объекта

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
    Employee sam {"Sam", "Company"};
    Person* psam {&sam};
    Employee* pemployee {static_cast<Employee*>(psam)};
    pemployee->print();

    Person tom {"Tom"};
    Person* ptom {&tom};
    Employee* pemployee1 {static_cast<Employee*>(ptom)};
    pemployee1->print();

    return 0;
}
```

```
{name: Sam, company: Company}
{name: Tom}
```

---
[Преобразование типов](https://metanit.com/cpp/tutorial/5.25.php)