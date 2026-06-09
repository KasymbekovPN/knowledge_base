---
tags:
  - programming-language
  - cpp
  - syntax
  - class
  - oop
  - constructor
  - inheritance
---
[[__cpp syntax oop inheritance__|<==]]

Если есть полное соответствие по параметрам между двумя классами, то можно не определять отдельный конструктор для Employee, а подключить конструктор базового класса.

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
        << ", age: "  << age
        << "}" << std::endl;
}

class Employee: public Person{

public:
    using Person::Person;
};

int main(int argc, char const *argv[]) {
    Person tom {"Tom", 42};
    tom.print();

    Employee bob {"Bob", 43};
    bob.print();

    return 0;
}
```

```
{name: Tom, age: 42}
{name: Bob, age: 43}
```

Здесь в классе _Employee_ подключаем конструктор базового класса с помощью ключевого слова _using_.

Таким образом, класс _Employee_ фактически будет иметь тот же конструктор, что и _Person_ с теми же двумя параметрами. И этот конструктор мы также можем вызвать для создания объекта _Employee_.

---
[Наследование](https://metanit.com/cpp/tutorial/5.10.php)