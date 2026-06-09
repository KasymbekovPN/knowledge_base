---
tags:
  - programming-language
  - cpp
  - syntax
  - operator
  - overloading
  - operator-overloading
---
[[__cpp syntax oop__|<=]]

__Перегрузка операторов__ (__operator overloading__) позволяет определить для объектов классов встроенные операторы, такие как `+`, `-`, `*` и т.д. Для определения оператора для объектов своего класса, необходимо определить функцию, название которой содержит слово _operator_ и символ перегружаемого оператора. Функция оператора может быть определена как член класса, либо вне класса.

Перегрузить можно только те операторы, которые уже определены в __C++__. _Создать новые операторы нельзя_. Также нельзя изменить количество операндов, их ассоциативность, приоритет.

Если функция оператора определена как отдельная функция и не является членом класса, то количество параметров такой функции совпадает с количеством операндов оператора. Если оператор принимает два операнда, то первый операнд передается первому параметру функции, а второй операнд - второму параметру. При этом как минимум один из параметров должен представлять тип класса.

`Определение операторов в виде функций членов`
```cpp
// unary
class_type& operator op();

// binary
return_type operator op(class_type right_operand);
```

`Определение операторов в виде простых функций`
```cpp
// unary
class_type& operator op(class_type &instance);

// binary
return_type operator op(const class_type& left_operand, type right_operand);

// binary (alt)
return_type operator op(type left_operand, const class_type& right_operand);
```

```cpp
#include <iostream>

class Counter {

private:
    int value;

public:
    Counter(int);
    int getValue() const;
    void print() const;
    Counter operator + (const Counter&) const;
    int operator + (int) const;
};

Counter::Counter(int value): value(value) {}

int Counter::getValue() const {
    return value;
}

void Counter::print() const {
    std::cout
        << "{value: " << getValue()
        << "}" << std::endl;
}

Counter Counter::operator + (const Counter& instance) const {
    return Counter {value + instance.getValue()};
}

int Counter::operator + (int other) const {
    return getValue() + other;
}

Counter operator * (const Counter&, const Counter&);

int main(int argc, char const *argv[]) {
    auto c0 = Counter{11};
    auto c1 = Counter{12};
    auto c2 = Counter{13};

    auto r0 = c0 + c1;
    r0.print();

    auto r1 = c2 + 100;
    std::cout << "value <= " << r1 << std::endl;

    auto r2 = c1 * c2;
    r2.print();

    return 0;
}

Counter operator * (const Counter& c0, const Counter& c1) {
    return Counter {c0.getValue() * c1.getValue()};
}
```

```
{value: 23}
value <= 113
{value: 156}
```

Какие операторы где переопределять? 

Операторы присвоения, индексирования ([]), вызова (()), доступа к члену класса по указателю (->) следует определять в виде функций-членов класса. 

Операторы, которые изменяют состояние объекта или непосредственно связаны с объектом (инкремент, декремент), обычно также определяются в виде функций-членов класса. 

Операторы выделения и удаления памяти (`new new[] delete delete[]`) определяются только в виде функций, которые не являются членами класса. 

Все остальные операторы можно определять как отдельные функции, а не члены класса.

- [[comparison operators overloading]]
- [[comparison operators overloading as default]]
- [[assignment operators overloading]]
- [[unary operators overloading]]
- [[inc dec operators overloading]]
- [[left shift operators overloading]]
- [[subscript operator overloading]]

---
[Перегрузка операторов](https://metanit.com/cpp/tutorial/5.14.php)
[Default comparison](https://en.cppreference.com/w/cpp/language/default_comparisons)
[Оператор индексирования](https://metanit.com/cpp/tutorial/5.20.php)