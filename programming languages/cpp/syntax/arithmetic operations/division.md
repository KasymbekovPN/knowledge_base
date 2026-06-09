---
tags:
  - programming-language
  - syntax
  - operation
  - arithmetic-operation
  - division
---
[[__cpp syntax arithmetic operations__|<=]]

Операция деления (__/__) возвращает частное двух чисел. 

Если в операции участвуют два целых числа, то дробная часть (при ее наличии) будет отбрасываться, даже если результат присваивается переменной _float_ или _double_.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int a {5};
    int b {10};
    double c {123.4};
    double d {56.7};

    int ab {a / b};
    double bc {b / c};
    double cd {c / d};

    cout << "ab <= " << ab << endl;
    cout << "bc <= " << bc << endl;
    cout << "cd <= " << cd << endl;

    return 0;
}
```

```
ab <= 0
bc <= 0.0810373
cd <= 2.17637
```

---
[Арифметические операции|metanit.com](https://metanit.com/cpp/tutorial/2.6.php)