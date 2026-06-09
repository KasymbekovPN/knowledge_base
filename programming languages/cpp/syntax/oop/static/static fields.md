---
tags:
  - programming-language
  - cpp
  - syntax
  - oop
  - static
---
[[__cpp static syntax oop__|<==]]

Статические переменные обычно применяются для хранения значений, специфичных для класса, для всех объектов класса в целом. То есть статические поля хранят состояние всего класса. Статическая переменная определяется только один раз и будет существовать, даже если объекты класса не были созданы.

Показательным примером статических переменных являются различные счетчики. 

```cpp
#include <iostream>

class Person {

private:
    static inline unsigned count {};

    std::string name;
    unsigned age;

public:
    Person(std::string, unsigned);
    void print_count();
};

Person::Person(std::string name, unsigned age): name(name), age(age) {
    count++;
}

void Person::print_count() {
    std::cout << "Count <= " << count << std::endl;
}

int main(int argc, char const *argv[]) {
    Person tom {"Tom", 42};
    tom.print_count();

    Person bob {"Bob", 43};
    tom.print_count();

    Person sam {"Sam", 44};
    tom.print_count();

    return 0;
}
```

```
Count <= 1
Count <= 2
Count <= 3
```

После _static_ идет ключевое слово _inline_. Это ключевое слово в принципе необязательно для статических переменных и необходимо конкретно в данном случае для инициализации переменной _count_. В данном случае нулем.

---
[Статические члены класса](https://metanit.com/cpp/tutorial/5.7.php)