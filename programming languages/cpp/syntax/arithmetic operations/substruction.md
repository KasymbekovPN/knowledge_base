---
tags:
  - programming-language
  - cpp
  - syntax
  - operation
  - arithmetic-operation
  - substruction
---
[[__cpp syntax arithmetic operations__|<=]]

Операция вычитания (__-__) возвращает разность двух чисел.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int a {10};
    int b {7};
    int c {a - b};
    int d {c - 100};

    cout << "a <= " << a << endl;
    cout << "b <= " << b << endl;
    cout << "c <= " << c << endl;
    cout << "d <= " << d << endl;

    return 0;
}
```

```
a <= 10
b <= 7
c <= 3
d <= -97
```

---
[Арифметические операции|metanit.com](https://metanit.com/cpp/tutorial/2.6.php)