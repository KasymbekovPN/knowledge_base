---
tags:
  - programming-language
  - cpp
  - syntax
  - comparison
  - operator-overloading
---
[[__cpp syntax oop operator overloading__|<=]]

Если речь идет о простом сравнении на основе полей класса, то для операторов `==` и `!=` проще использовать специальный оператор _default_ (C++20). 

```cpp
#include <iostream>

class Counter {

private:
    int value;

public:
    Counter(int);
    void print() const;
    bool operator == (const Counter& counter) const = default;
    bool operator != (const Counter& counter) const = default;
};

Counter::Counter(int value): value{value} {}

void Counter::print() const {
    std::cout
        << "{value: " << value
        << "}" << std::endl;
}

void print(std::string, bool);

int main(int argc, char const *argv[]) {
    Counter c0 {12};
    Counter c1 {42};

    c0.print();
    c1.print();

    print("==", c0 == c1);
    print("!=", c0 != c1);

    return 0;
}

void print(std::string op, bool expr) {
    std::cout
        << "c0 " << op << " c1 => "
        << std::boolalpha
        << expr
        << std::noboolalpha
        << std::endl;
}
```

```
{value: 12}
{value: 42}
c0 == c1 => false
c0 != c1 => true
```

---
[Перегрузка операторов](https://metanit.com/cpp/tutorial/5.14.php)
[Default comparison](https://en.cppreference.com/w/cpp/language/default_comparisons)