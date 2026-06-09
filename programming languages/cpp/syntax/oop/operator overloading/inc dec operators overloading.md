---
tags:
  - programming-language
  - cpp
  - syntax
  - increment
  - decrement
  - operator-overloading
---
[[__cpp syntax oop operator overloading__|<=]]

```cpp
#include <iostream>

class Counter {

private:
    int value;

public:
    Counter(int);
    void print() const;
    // prefix
    Counter& operator++();
    Counter& operator--();
    // postfix
    Counter operator++(int);
    Counter operator--(int);
};

Counter::Counter(int value): value{value} {}

void Counter::print() const {
    std::cout
        << "{value: " << value
        << "}" << std::endl;
}

Counter& Counter::operator++() {
    value++;
    return *this;
}

Counter& Counter::operator--() {
    value--;
    return *this;
}

Counter Counter::operator++(int) {
    Counter copy {*this};
    ++(*this);
    return copy;
}

Counter Counter::operator--(int) {
    Counter copy {*this};
    --(*this);
    return copy;
}

int main(int argc, char const *argv[]) {
    Counter c0 {42};
    Counter c1 = c0++;
    c1.print();

    ++c1;
    c1.print();

    return 0;
}
```

```
{value: 42}
{value: 43}
```

Префиксные операторы должны возвращать ссылку на текущий объект.

Постфиксные операторы должны возвращать значение объекта до инкремента, то есть предыдущее состояние объекта. Поэтому постфиксная форма возвращает копию объекта до инкремента.

---
[Перегрузка операторов](https://metanit.com/cpp/tutorial/5.14.php)