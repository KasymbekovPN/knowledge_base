---
tags:
  - programming-language
  - cpp
  - syntax
  - oop
  - class
  - function
---
[[__cpp syntax oop__|<==]]

В языке __C++__ можно разделять объявление и определение функций в том числе по отношению к функциям, которые создаются в классах. Для этого используется выражение
```cpp
type class_name::function_name(parameters) {
	// ...
}
```

```cpp
#include <iostream>

class Person {

private:
    std::string name;
    unsigned age;

public:
    Person(std::string _name, unsigned _age);
    Person(std::string _name);
    void print();
};

Person::Person(std::string _name, unsigned _age):
	name(_name), age(_age) {}

Person::Person(std::string _name): Person(_name, 42) {}

void Person::print() {
    std::cout
        << "{name: " << name
        << ", age: " << age
        << "}" << std::endl;
}

int main(int argc, char const *argv[]) {
    Person tom {"Tom", 24};
    tom.print();

    Person bob {"Bob"};
    bob.print();

    return 0;
}
```

Теперь функции класса _Person_ (в данном случае _конструкторы_ и функция _print_) в самом классе имеют только объявления.

Реализации функций размещены вне класса _Person_.

---
[Объявление и определение функций класса](https://metanit.com/cpp/tutorial/5.3.php)