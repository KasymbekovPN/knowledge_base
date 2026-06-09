---
tags:
  - programming-language
  - cpp
  - syntax
  - operator-overloading
  - explicit
---
[[__cpp syntax oop type conversion__|<=]]

Неявные преобразования не всегда могут быть желательны. В этом случае их можно отключить, определив функцию оператора с помощью ключевого слова _explicit_.

```cpp
#include <iostream>

class Counter {

private:
    int value;

public:
    Counter(int);
    explicit operator int() const;
};

Counter::Counter(int value): value{value} {}

Counter::operator int() const {
    return value;
}

int main(int argc, char const *argv[]) {
    Counter counter {42};

    int i0 = static_cast<int>(counter);
    // int i1 = counter; // <= Error

    return 0;
}
```

```
.\type_conversion_op_explicit_cast.cpp:23:9: error: no viable conversion from 'Counter' to 'int'
   23 |     int i1 = counter;
      |         ^    ~~~~~~~
.\type_conversion_op_explicit_cast.cpp:10:14: note: explicit conversion function is not a candidate
   10 |     explicit operator int() const;
      |              ^
```

---
[Операторы преобразования типов](https://metanit.com/cpp/tutorial/5.15.php)