---
tags:
  - programming-language
  - syntax
  - operation
  - arithmetic-operation
  - addition
---
[[__cpp syntax arithmetic operations__|<=]]

Операция сложения (__+__) возвращает сумму двух чисел.  В примере результат операций применяется для инициализации переменных, но также можно использовать операцию присвоения для установки значения переменных.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int a {10};
    int b {5};
    int c {a + b};
    int d {c + 10};

    cout << "a <= " << a << endl;
    cout << "b <= " << b << endl;
    cout << "c <= " << c << endl;
    cout << "d <= " << d << endl;

    return 0;
}
```

```
a <= 10
b <= 5
c <= 15
d <= 25
```

---
[Арифметические операции|metanit.com](https://metanit.com/cpp/tutorial/2.6.php)