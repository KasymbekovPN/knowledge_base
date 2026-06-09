---
tags:
  - programming-language
  - cpp
  - oop
  - virtual
---
[[__cpp syntax oop__|<==]]

При вызове функции программа должна определять, с какой именно реализацией функции соотносить этот вызов, то есть связать вызов функции с самой функцией. В __С++__ есть два типа связывания - __статическое__ и __динамическое__.

Когда вызовы функций фиксируются до выполнения программы на этапе компиляции, это называется статическим связыванием (__static binding__), либо ранним связыванием (__early binding__). При этом вызов функции через указатель определяется исключительно типом указателя, а не объектом, на который он указывает.

```cpp
#include <iostream>

class Person {

private:
    std::string name;

public:
    Person(std::string name);
    void print() const;
};

Person::Person(std::string name): name{name}{}

void Person::print() const {
    std::cout << "name <= " << name << std::endl;
}

class Employee: public Person {

private:
    std::string company;

public:
    Employee(std::string, std::string);
    void print() const;
};

Employee::Employee(std::string name, std::string company):
    Person{name},
    company{company} {}

void Employee::print() const {
    Person::print();
    std::cout << "company <= " << company << std::endl;
}

int main(int argc, char const *argv[]) {
    Person tom {"Tom"};
    
    Person* p {&tom};
    p->print();

    Employee bob {"Bob", "Company"};
    p = &bob;
    p->print();

    return 0;
}
```

```
name <= Tom
name <= Bob
```

Другой тип связывания представляет __динамическое связывание__ (__dynamic binding__), еще называют поздним связыванием (__late binding__), которое позволяет на этапе выполнения решать, функцию какого типа вызвать. Для этого в языке __С++__ применяют __виртуальные__ функции. Для определения виртуальной функции в базовом классе функция определяется с ключевым словом _virtual_. Причем данное ключевое слово можно применить к функции, если она определена внутри класса. А производный класс может переопределить ее поведение.

```cpp
#include <iostream>

class Person {

private:
    std::string name;

public:
    Person(std::string);
    virtual void print() const;
};

Person::Person(std::string name): name(name) {}

void Person::print() const {
    std::cout << "name <= " << name << std::endl;
}

class Employee: public Person{

private:
    std::string company;

public:
    Employee(std::string, std::string);
    void print() const;
};

Employee::Employee(std::string name, std::string company):
	Person{name},
	company{company} {}

void Employee::print() const {
    Person::print();
    std::cout << "Works in " << company << std::endl;
}

int main(int argc, char const *argv[]) {
    Person tom {"Tom"};
    Person* p {&tom};
    p->print();

    p = new Employee{"Bob", "Company"};
    p->print();

    return 0;
}
```

```
name <= Tom
name <= Bob
Works in Company
```

Таким образом, базовый класс Person определяет виртуальную функцию print, а производный класс Employee переопределяет ее.

При определении виртуальных функций есть ряд ограничений. Чтобы функция попадала под динамическое связывание, в производном классе она должна иметь тот же самый набор параметров и возвращаемый тип, что и в базовом классе. Например, если в базовом классе виртуальная функция определена как константная, то в производном классе она тоже должна быть константной. Если же функция имеет разный набор параметров или несоответствие по константности, то мы будем иметь дело со скрытием функций, а не переопределением. И тогда будет применяться статическое связывание.

Также статические функции не могут быть виртуальными.

- [[key word override]]
- [[principle of virtual method execution]]
- [[prohibition of overriding]]

---
[Виртуальные функции](https://metanit.com/cpp/tutorial/5.11.php)