---
tags:
  - programming-language
  - cpp
  - syntax
  - operator-overloading
  - subscript
  - subscript-operator
---
[[__cpp syntax oop operator overloading__|<=]]

Оператор индексирования `[]` (`subscript operator`) позволяет интерпретировать объект как массив или как контейнер других объектов и позволяет выбирать из объекта отдельные элементы. Функция оператора [] должна принимать в качестве аргумента условный индекс, по которому в объекте-контейнере можно найти нужный элемент, например, _unsigned_, _string_.  

```cpp
#include <iostream>
#include <string>

class Person {

private:
    std::string name;
    std::string company;
    unsigned age;

public:
    Person(std::string, unsigned, std::string);
    void print() const;
    std::string operator[](unsigned) const;
};

Person::Person(std::string name, unsigned age, std::string company):
    name{name},
    age{age},
    company{company} {}

void Person::print() const {
    std::cout
        << "{name: " << name
        << ", age: " << age
        << ", company: " << company
        << "}" << std::endl;
}

std::string Person::operator[](unsigned index) const {
    switch (index) {
        case 0: return name;
        case 1: return std::to_string(age);
        case 2: return company;
        default: return "Bad index";
    }
}

int main(int argc, char const *argv[]) {
    Person p {"Tom", 42, "Company"};
    p.print();

    const unsigned INDEXES[] {0, 1, 2, 3};
    for (auto idx : INDEXES) {
        std::cout << "idx: " << idx << " <=> " << p[idx] << std::endl;
    }

    return 0;
}
```

```
{name: Tom, age: 42, company: Company}
idx: 0 <=> Tom
idx: 1 <=> 42
idx: 2 <=> Company
idx: 3 <=> Bad index
```

---
[Оператор индексирования](https://metanit.com/cpp/tutorial/5.20.php)