---
tags:
  - programming-language
  - syntax
  - operation
  - arithmetic-operation
  - increment
---
[[__cpp syntax arithmetic operations__|<=]]

Префиксный инкремент увеличивает значение переменной на единицу и полученный результат используется как значение выражения `++x`.

Постфиксный инкремент увеличивает значение переменной на единицу, но значением выражения `x++` будет то, которое было до увеличения на единицу.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int a {7};
    int b {++a};
    int c {8};
    int d {c++};

    cout << "a <= " << a << endl;
    cout << "b <= " << b << endl;
    cout << "c <= " << c << endl;
    cout << "d <= " << d << endl;

    return 0;
}
```

```
a <= 8
b <= 8
c <= 9
d <= 8
```

---
[Арифметические операции|metanit.com](https://metanit.com/cpp/tutorial/2.6.php)
