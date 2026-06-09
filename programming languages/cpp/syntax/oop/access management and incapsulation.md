---
tags:
  - programming-language
  - cpp
  - syntax
  - oop
  - access
  - incapsulation
---
[[__cpp syntax oop__|<==]]

Класс может определять различное состояние, различные функции. Однако не всегда желательно, чтобы к некоторым компонента класса был прямой доступ извне. Для разграничения доступа к различным компонентам класса применяются спецификаторы доступа

Спецификатор _public_ делает члены класса - поля и функции открытыми, доступными из любой части программы. 

```cpp
#include <iostream>

class Person {

private:
    std::string private_name;
    unsigned private_age;

public:
    std::string public_name;
    unsigned public_age;

    Person(std::string p_name, unsigned p_age):
        private_name(p_name),
        public_name(p_name),
        private_age(p_age),
        public_age(p_age) {};

    void print() {
        std::cout
            << "{private_name: " << private_name
            << ", public_name: " << public_name
            << ", private_age: " << private_age
            << ", public_age: " << public_age
            << "}" << std::endl;
    }
};

int main(int argc, char const *argv[]) {
    Person tom {"Tom", 42};
    tom.public_name += " PUB";
    tom.public_age++;
    // tom.private_name += "PRI"; //< Error
    // tom.private_age += 2; //< Error

    tom.print();

    return 0;
}
```

```
{private_name: Tom, public_name: Tom PUB, private_age: 42, public_age: 43}
```

```
.\simple_access_demo.cpp:32:9: error: 'private_name' is a private member of 'Person'
   32 |     tom.private_name += "PRI"; //< Error
      |         ^
.\simple_access_demo.cpp:6:17: note: declared private here
    6 |     std::string private_name;
      |                 ^
.\simple_access_demo.cpp:33:9: error: 'private_age' is a private member of 'Person'
   33 |     tom.private_age += 2; //< Error
      |         ^
.\simple_access_demo.cpp:7:14: note: declared private here
    7 |     unsigned private_age;
      |              ^
```

То есть в данном случае поля _public_name_ и _public_age_ и функция _print_ являются открытыми, общедоступными, и мы можем обращаться к ним во внешнем коде. 

Однако с помощью другого спецификатора _private_ мы можем скрыть реализацию членов класса, то есть сделать их закрытыми, `инкапсулировать` внутри класса.   Это делается при помощи модификатора _private_.

Все компоненты, которые определяются после спецификатора _private_ и идут до спецификатора _public_, являются закрытыми, приватными.  К ним можно обращаться только внутри класса.

Если для каких-то компонентов отсутствует спецификатор доступа, то по умолчанию применяется спецификатор _private_. 

#### Опосредование доступа

Можно определить специальные функции, через которые будем контролировать доступ к состоянию класса.

```cpp
#include <iostream>

class Person {

private:
    std::string name;
    unsigned age;

public:
    Person(std::string p_name, unsigned p_age): name(p_name){
        age = p_age > 0 && p_age < 110 ? p_age : 18;
    }

    void setName(std::string p_name){
        name = p_name;
    }

    std::string getName() {
        return name;
    }

    void setAge(unsigned p_age) {
        if (p_age > 0 && p_age < 110) {
            age = p_age;
        }
    }

    unsigned getAge() {
        return age;
    }

    void print() {
        std::cout
            << "{name: " << name
            << ", age: " << age
            << "}" << std::endl;
    }
};

int main(int argc, char const *argv[]) {
    Person tom {"Tom", 42};
    tom.print();

    tom.setName(tom.getName() + " !!!");
    tom.setAge(tom.getAge() + 1);
    tom.print();

    return 0;
}
```

```
{name: Tom, age: 42}
{name: Tom !!!, age: 43}
```

---
[Управление доступом. Инкапсуляция](https://metanit.com/cpp/tutorial/5.4.php)