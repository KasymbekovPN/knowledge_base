---
tags:
  - programming-language
  - syntax
  - operation
  - arithmetic-operation
  - modulo
---
[[__cpp syntax arithmetic operations__|<=]]

Операция получения остатка от целочисленного деления (__%__)

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int a {26};
    int b {5};
    int c {4};

    int ab {a % b};
    int cb {c % b};

    cout << "ab < = " << ab << endl;
    cout << "cb < = " << cb << endl;

    return 0;
}
```

```
ab < = 1
cb < = 4
```

---
[Арифметические операции|metanit.com](https://metanit.com/cpp/tutorial/2.6.php)
