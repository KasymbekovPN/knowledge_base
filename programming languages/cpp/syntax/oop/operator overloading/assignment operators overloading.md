---
tags:
  - programming-language
  - cpp
  - syntax
  - operator
  - comparison
---
[[__cpp syntax oop operator overloading__|<=]]

Компилятор по умолчанию компилирует для типов оператор присваивания, благодаря чему мы можем присваивать значения некоторого типа переменным/параметрам/константам этого типа. Создаваемый по умолчанию оператор присваивания просто копирует все переменные-члены класса одну за другой (в том порядке, в котором они объявлены в определении класса).

```cpp
#include <iostream>

class Counter0 {

private:
    inline static int inner_counter {0};
    int current_counter;
    int value;

public:
    Counter0(int);
    void setValue(const int);
    void print() const;
};

class Counter1 {

private:
    inline static int inner_counter {0};
    int current_counter;
    int value;

public:
    Counter1(int);
    void print() const;
    Counter1& operator=(const Counter1&);
    Counter1& operator+=(const Counter1&);
    Counter1& operator-=(const Counter1&);
};

Counter0::Counter0(int value): value{value} {
    current_counter = inner_counter++;
}

void Counter0::setValue(const int value) {
    this->value = value;
}

void Counter0::print() const {
    std::cout
        << "{value: " << value
        << ", current counter: " << current_counter
        << "}" << std::endl;
}

Counter1::Counter1(int value): value{value} {
    current_counter = inner_counter++;
}

void Counter1::print() const {
    std::cout
        << "{value: " << value
        << ", current counter: " << current_counter
        << "}" << std::endl;
}

Counter1& Counter1::operator=(const Counter1& other) {
    if (&other != this) {
        value = other.value;
    }
    return *this;
}

Counter1& Counter1::operator+=(const Counter1& other) {
    value += other.value;
    return *this;
}

Counter1& Counter1::operator-=(const Counter1& other) {
    value -= other.value;
    return *this;
}

int main(int argc, char const *argv[]) {
    Counter0 original_counter0 {42};
    Counter0 copy_counter0 = original_counter0;
    original_counter0.print();
    copy_counter0.print();

    copy_counter0.setValue(123);
    original_counter0.print();
    copy_counter0.print();

    Counter1 c0 {12};
    Counter1 c1 {21};
    c0.print();
    c1.print();

    c0 = c1;
    c0.print();

    c0 += c1;
    c0.print();

    return 0;
}
```

```
{value: 42, current counter: 0}
{value: 42, current counter: 0}
{value: 42, current counter: 0}
{value: 123, current counter: 0}
{value: 12, current counter: 0}
{value: 21, current counter: 1}
{value: 21, current counter: 0}
{value: 42, current counter: 0}
```

---
[Перегрузка операторов](https://metanit.com/cpp/tutorial/5.14.php)
[Переопределение оператора присваивания](https://metanit.com/cpp/tutorial/5.21.php)