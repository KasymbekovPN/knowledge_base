---
tags:
  - programming-language
  - cpp
  - syntax
  - oop
  - static
---
[[__cpp static syntax oop__|<==]]

Статические функции также принадлежат классу в целом и не зависят от любого отдельного объекта класса. Обычно статические функции-члены используются для работы со статическими переменными. 

Для определения статической функции перед ней также указывается ключевое слово _static_.

К статической функции можно обратиться и через класс, и через экземпляр.

```cpp
#include <iostream>

class Person {

private:
    static inline unsigned count {};

    std::string name;
    unsigned age;

public:
    Person(std::string, unsigned);
    void print();
    static void print_count();
};

Person::Person(std::string name, unsigned age): name(name), age(age) {
    count++;
}

void Person::print() {
    std::cout
        << "{name: " << name
        << ", age: " << age
        << ", count: " << count
        << "}" << std::endl;
}

void Person::print_count() {
    std::cout << "Count <= " << count << std::endl;
}

int main(int argc, char const *argv[]) {
    Person tom {"Tom", 42};
    Person bob {"Bob", 43};
    Person sam {"Sam", 44};

    tom.print();
    bob.print();
    sam.print();

    tom.print_count();
    Person::print_count();

    return 0;
}
```

```
{name: Tom, age: 42, count: 3}
{name: Bob, age: 43, count: 3}
{name: Sam, age: 44, count: 3}
Count <= 3
Count <= 3
```

---
[Статические члены класса](https://metanit.com/cpp/tutorial/5.7.php)