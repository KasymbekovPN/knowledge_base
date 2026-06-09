---
tags:
  - programming-language
  - cpp
  - syntax
  - unary-operator
  - operator-overloading
---
[[__cpp syntax oop operator overloading__|<=]]

__Унарные операции__ обычно возвращают новый объект, созданный на основе имеющегося.

Здесь операция унарного минуса возвращает новый объект _Counter_, значение value в котором фактически равно значению value текущего объекта, умноженного на `-1`.

```cpp
#include <iostream>

class Counter {

private:
    int value;

public:
    Counter(int);
    void print() const;
    Counter operator - () const;
};

Counter::Counter(int value): value{value} {}

void Counter::print() const {
    std::cout
        << "{value: " << value
        << "}" << std::endl;
}

Counter Counter::operator - () const {
    return Counter{-value};
}

int main(int argc, char const *argv[]) {
    Counter c0{42};
    c0.print();

    Counter c1 = -c0;
    c1.print();

    return 0;
}
```

```
{value: 42}
{value: -42}
```

---
[Перегрузка операторов](https://metanit.com/cpp/tutorial/5.14.php)