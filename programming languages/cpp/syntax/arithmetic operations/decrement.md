---
tags:
  - programming-language
  - syntax
  - operation
  - arithmetic-operation
  - decrement
---
[[__cpp syntax arithmetic operations__|<=]]

Префиксный декремент уменьшает значение переменной на единицу, и полученное значение используется как значение выражения `--x`.

Постфиксный декремент уменьшает значение переменной на единицу, но значением выражения `x--` будет то, которое было до уменьшения на единицу.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int a {7};
    int b {--a};
    int c {8};
    int d {c--};

    cout << "a <= " << a << endl;
    cout << "b <= " << b << endl;
    cout << "c <= " << c << endl;
    cout << "d <= " << d << endl;

    return 0;
}
```

```
a <= 6
b <= 6
c <= 7
d <= 8
```

---
[Арифметические операции|metanit.com](https://metanit.com/cpp/tutorial/2.6.php)
