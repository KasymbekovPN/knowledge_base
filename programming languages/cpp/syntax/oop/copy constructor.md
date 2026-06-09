---
tags:
  - programming-language
  - cpp
  - syntax
  - oop
  - constructor
  - copy-constructor
  - copy
---
[[__cpp syntax oop__|<==]]

По умолчанию компилятор при компиляции классов генерирует специальный конструктор - конструктор копирования, который позволяет создать объект на основе другого объекта (по сути копирует объект). Конструктор копирования по умолчанию копирует значения полей объекта в новый объект. 

```cpp
#include <iostream>

class Person {

private:
    std::string name;
    unsigned age;

public:
    Person(std::string _name, unsigned _age);
    void print();
};

Person::Person(std::string _name, unsigned _age):
	name(_name), age(_age) {}

void Person::print() {
    std::cout
        << "{name: " << name
        << ", age: " << age
        << "}" << std::endl;
}

int main(int argc, char const *argv[]) {
    Person tom {"Tom", 42};
    Person tomas {tom};

    tom.print();
    tomas.print();

    return 0;
}
```

```
{name: Tom, age: 42}
{name: Tom, age: 42}
```

`Person tomas {tom};`  - вызов конструктора копирования.

__Конструктор копирования__ - замечательная вещь, когда нам надо создать один объект на основе другого, однако данный конструктор имеет свои недостатки. Например, если поле представляет указатель, то копируется адрес. В итоге поля обоих объектов будут указывать на один и тот же адрес в памяти. Соответственно если мы захотим изменить значение для одного объекта, это значение также изменится и для другого объекта. И в этом случае мы можем определить свой конструктор копирования.

#### Создание конструктора копирования

Конструктор копирования должен принимать в качестве параметра объект того же класса. Причем параметр лучше принимать по ссылке, потому что при передаче по значению компилятор будет создавать копию объекта. А для создания копия объекта будет вызываться конструктор копирования, что приведет бесконечной рекурсии. 

```cpp
#include <iostream>

class Person {

private:
    std::string name;
    unsigned age;

public:
    Person(std::string _name, unsigned _age);
    Person(const Person& other);
    void print();
};

Person::Person(std::string _name, unsigned _age):
	name(_name),
	age(_age) {}

Person::Person(const Person& other):
	name("Copy of " + other.name),
	age(other.age + 1) {}

void Person::print() {
    std::cout
        << "{name: " << name
        << ", age: " << age
        << "}" << std::endl;
}

int main(int argc, char const *argv[]) {
    Person tom {"Tom", 42};
    tom.print();

    Person tomas {tom};
    tomas.print();

    return 0;
}
```

```
{name: Tom, age: 42}
{name: Copy of Tom, age: 43}
```

Здесь конструктор копирования принимает константную ссылку на объект __Person__ и присваивает значения его полей соответствующим полям текущего объекта. 

#### Удаление конструктора копирования

Конструктор копирования не всегда может быть нужен. И его можно удалить с помощью оператора _delete_:

```cpp
#include <iostream>

class Person {

private:
    std::string name;
    unsigned age;

public:
    Person(std::string _name, unsigned _age);
    Person(const Person& other) = delete;
    void print();
};

Person::Person(std::string _name, unsigned _age):
	name(_name),
	age(_age) {}

void Person::print() {
    std::cout
        << "{name: " << name
        << ", age: " << age
        << "}" << std::endl;
}

int main(int argc, char const *argv[]) {
    Person tom {"Tom", 42};
    // Person tomas {tom}; // Error

    return 0;
}
```

```
.\deleted_copy_constructor.cpp:26:12: error: call to deleted constructor of 'Person'
   26 |     Person tomas {tom};
      |            ^     ~~~~~
.\deleted_copy_constructor.cpp:11:5: note: 'Person' has been explicitly marked deleted here
   11 |     Person(const Person& other) = delete;
      |     ^
```

---
[Конструктор копирования](https://metanit.com/cpp/tutorial/5.17.php)