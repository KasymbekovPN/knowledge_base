---
tags:
  - programming-language
  - cpp
  - syntax
  - operation
  - arithmetic-operation
  - multiplication
---
[[__cpp syntax arithmetic operations__|<=]]

Операция умножения (__\*__) возвращает произведение двух чисел.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int a {10};
    int b {7};
    int c {a * b};
    int d {-3 * c};

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
c <= 70
d <= -210
```

---
[Арифметические операции|metanit.com](https://metanit.com/cpp/tutorial/2.6.php)