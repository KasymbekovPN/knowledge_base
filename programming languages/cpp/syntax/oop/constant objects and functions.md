---
tags:
  - programming-language
  - cpp
  - syntax
  - class
  - constants
---
[[__cpp syntax oop__|<==]]

#### Константные объекты

Объекты классов также могут представлять константы.

```cpp
#include <iostream>

class Person {

public:
    std::string name;
    unsigned age;

    Person(std::string _name, unsigned _age);
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
    const Person tom {"Tome", 42};
    std::cout
        << "{name: " << tom.name
        << ", age: " << tom.age
        << "}" << std::endl;

    // tom.age++; // Error
    // tom.print(); // Error
  
    return 0;
}
```

```
{name: Tome, age: 42}
```

```
.\simple_const_object.cpp:28:12: error: cannot assign to variable 'tom' with const-qualified type 'const Person'
   28 |     tom.age++; // Error
      |     ~~~~~~~^
.\simple_const_object.cpp:23:18: note: variable 'tom' declared const here
   23 |     const Person tom {"Tome", 42};
      |     ~~~~~~~~~~~~~^~~~~~~~~~~~~~~~
```

```
.\simple_const_object.cpp:29:5: error: 'this' argument to member function 'print' has type 'const Person', but function is not marked const
   29 |     tom.print(); // Error
      |     ^~~
.\simple_const_object.cpp:15:14: note: 'print' declared here
   15 | void Person::print() {
      |              ^
```

Но при работе с константными объектами мы можем получить данные их полей, но изменить их не можем.

Так же невозможно выполнение не константных  функций.

#### Функции константного объекта

Константность объекта накладывает некоторые ограничения на вызов его функций. 

Потому что в любой функции класса теоретически можно изменять его поля, а компилятор не может определить, меняется ли значение в функции или нет. Поэтому одинаково отказывается компилировать и те функции, которые меняют состояние объекта, и те функции, которые его не меняют.

Для константного объекта можно вызывать только константные функции. Для определения таких функций после списка параметров ставится ключевое слово _const_.

Константная функция может вызывать только константные функции класса.

```cpp
#include <iostream>

class Person {

private:
    std::string name;
    unsigned age;

public:
    Person(std::string _name, unsigned _age);
    void print() const;
};

Person::Person(std::string _name, unsigned _age):
	name(_name),
	age(_age) {}

void Person::print() const{
    std::cout
        << "{name: " << name
        << ", age: " << age
        << "}" << std::endl;
}

int main(int argc, char const *argv[]) {
    const Person tom {"Tome", 42};
    tom.print();

    return 0;
}
```

```
{name: Tome, age: 42}
```

#### Возвращение констант

Еще одно ограничение, связанное с константными функциями, состоит в том, что, если мы хотим возвратить из константной функции __указатель__ или __ссылку__, то они указатель должен указывать на константу, а ссылка должна быть константной.

```cpp
#include <iostream>

class Person {

private:
    std::string name;
    unsigned age;

public:
    Person(std::string _name, unsigned _age);
    const std::string& getName() const;
    const unsigned* getAge() const;
    void print() const;
};

Person::Person(std::string _name, unsigned _age):
	name(_name),
	age(_age) {}

const std::string& Person::getName() const {
    return name;
}

const unsigned* Person::getAge() const {
    return &age;
}

void Person::print() const{
    std::cout
        << "{name: " << name
        << ", age: " << age
        << "}" << std::endl;
}

int main(int argc, char const *argv[]) {
    const Person tom {"Tome", 42};
    std::cout
        << "{name: " << tom.getName()
        << ", age: " << *tom.getAge()
        << "}" << std::endl;

    return 0;
}
```

```
{name: Tome, age: 42}
```

#### Ключевое слово _mutable_

Ключевое слово _mutable_ дает возможность переменной быть изменяемой в рамках константного объекта.

```cpp
#include <iostream>

class Person {

public:
    std::string name;
    mutable unsigned age;

    Person(std::string _name, unsigned _age);
    void print() const;
};

Person::Person(std::string _name, unsigned _age):
	name(_name),
	age(_age) {}

void Person::print() const{
    std::cout
        << "{name: " << name
        << ", age: " << age
        << "}" << std::endl;
}

int main(int argc, char const *argv[]) {
    const Person tom {"Tome", 42};
    tom.age++;
    tom.print();

    return 0;
}
```

```
{name: Tome, age: 43}
```

---
[Константные объекты и функции](https://metanit.com/cpp/tutorial/5.18.php)