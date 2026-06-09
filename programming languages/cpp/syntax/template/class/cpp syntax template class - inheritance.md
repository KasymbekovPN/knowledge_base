---
tags:
  - programming-language
  - cpp
  - syntax
  - template
  - class
  - inheritance
---
[[_cpp syntax template class|<=]]

При наследовании класса на основе шаблона нам надо указать значения для параметров шаблона базового класса. И в данном случае мы можем также и производный класс определить как шаблон, и использовать его параметры при установке базового класса. Или же можно указать на этапе наследования специализацию базового класса.

```cpp
#include <iostream>

template<typename T>
class Person {

protected:
    T id;
    std::string name;

public:
    explicit Person(T id, std::string name) noexcept: id{id}, name{name} {}
    void print() const noexcept {
        std::cout
            << "{id: " << id
            << ", name: " << name
            << "}" << std::endl;
    }
};

template<typename T>
class Employee: Person<T>{

private:
    std::string company;

public:
    explicit Employee(T id,
				      std::string name,
				      std::string company) noexcept:
        Person<T>{id, name},
        company{company} {}

    void print() const noexcept {
        Person<T>::print();
        std::cout
            << Person<T>::name << " is working in "
            << company << std::endl;
    }
};

class UEmployee: Person<unsigned>{

private:
    std::string company;

public:
    explicit UEmployee(unsigned id,
				       std::string name,
				       std::string company) noexcept:
        Person{id, name},
        company{company} {}

    void print() const noexcept {
        Person::print();
        std::cout
            << name << " is working in "
            << company << std::endl;
    }
};

int main(int argc, char const *argv[]) {
    Employee<std::string> tom {"xyz", "Tom", "Company"};
    tom.print();

    UEmployee bob {123, "Bob", "Company"};
    bob.print();

    return 0;
}
```

```
{id: xyz, name: Tom}
Tom is working in Company
{id: 123, name: Bob}
Bob is working in Company
```

---
[Наследование и шаблоны классов](https://metanit.com/cpp/tutorial/9.4.php)