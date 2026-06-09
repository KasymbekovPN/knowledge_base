---
tags:
  - programming-language
  - cpp
  - syntax
  - inheritance
---
[[__cpp syntax oop__|<==]]

__Наследование__ (__inheritance__) представляет один из ключевых аспектов объектно-ориентированного программирования, который позволяет наследовать функциональность одного класса (базового класса) в другом - производном классе (derived class).

```cpp
#include <iostream>

class Person {

public:
    std::string name;
    unsigned age;

    void print();
};

void Person::print() {
    std::cout
        << "{name: " << name
        << ", age: " << age
        << "}" << std::endl;
}

class Employee: public Person {

public:
    std::string company;
};

int main(int argc, char const *argv[]) {
    Person tom;
    tom.name = "Tom";
    tom.age = 42;
    tom.print();

    Employee bob;
    bob.name = "Bob";
    bob.age = 43;
    bob.company = "Some company";
    bob.print();

    return 0;
}
```

```
{name: Tom, age: 42}
{name: Bob, age: 43}
```

Для установки отношения наследования после названия класса ставится двоеточие, затем идет спецификатор доступа и название класса, от которого мы хотим унаследовать функциональность. 

Спецификатор доступа позволяет указать, к каким членам класса производный класс будет иметь доступ. В данном случае используется спецификатор _public_ который позволяет использовать в производном классе все публичные члены базового класса. Если мы не используем модификатор доступа, то класс Employee ничего не будет знать о переменных _name_ и _age_ и функции _print_.

[[constructors on inheritance]]
[[connection of base class constructor]]
[[defining copy constructors]]
[[destructor inheritance]]
[[prohibited inhetitance]]

---
[Наследование](https://metanit.com/cpp/tutorial/5.10.php)