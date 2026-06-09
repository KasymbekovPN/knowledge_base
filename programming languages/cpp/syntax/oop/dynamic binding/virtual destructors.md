---
tags:
  - programming-language
  - cpp
  - syntax
  - oop
  - destructor
  - virtual
---
[[__cpp syntax oop dynamic binding__|<=]]

__Деструктор__ определяет логику удаления класса. При удалении объекта производного класса мы ожидаем, что будет выполняться деструктор производного, а затем и деструктор базового классов, что позволяет выполнить необходимую логику (например, освобождение выделенной памяти) для обоих классов. Однако в некоторых ситуациях такое может не сработать.

```cpp
#include <iostream>
#include <memory>

class Person {

private:
    std::string name;

public:
    Person(std::string);
    virtual ~Person();
};

Person::Person(std::string name): name{name} {}

Person::~Person() {
    std::cout << "Person deleted" << std::endl;
}

class Employee: public Person {

private:
    std::string company;
public:
    Employee(std::string, std::string);
    ~Employee();
};

Employee::Employee(std::string name, std::string company):
    Person{name}, company{company} {}

Employee::~Employee() {
    std::cout << "Employee deleted" << std::endl;
}

int main(int argc, char const *argv[]) {
    std::unique_ptr<Person> sam {
	    std::make_unique<Employee>("Sam", "Company")
	};
    return 0;
}
```

`Если деструктор Person виртуальный`
```
Employee deleted
Person deleted
```

`Если деструктор Person не виртуальный`
```
Person deleted
```

Здесь переменная _sam_ представляет smart-указатель `std::unique_ptr` на объект _Person_, который автоматически выделяет память для одного объекта _Employee_.

Для обоих классов определены деструкторы, который просто выводят строку на консоль. То есть мы ожидаем, что после завершения функции _main_ объект указателя _sam_ будет удален, и будут выполняться деструкторы классов _Employee_ и _Person_ (ведь у нас объект _Employee_). 

---
[Особенности динамического связывания](https://metanit.com/cpp/tutorial/5.27.php)