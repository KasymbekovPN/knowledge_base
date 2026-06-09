---
tags:
  - programming-language
  - cpp
  - data-types
  - static
  - cast
  - conversion
---

[[__cpp data types index__|<=]]

__С++__ является статически типизированным языком программирования. То есть если мы определили для переменной какой-то тип данных, то в последующем мы этот тип изменить не сможем. Соответственно переменная может получить значения только того типа, который она представляет. Однако нередко возникает необходимость присвоить переменной значения каких-то других типов. И в этом случае применяются преобразования типов.

Ряд преобразований компилятор может производить неявно, то есть автоматически, но не всегда.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    unsigned int ui0 {25};
    // unsigned int ui1 {-25}; // error

    cout << "ui0 <= " << ui0 << endl;
    // cout << "ui1 <= " << ui1 << endl

    return 0;
}
```

Здесь 25(_int_) неявно конвертируется в _unsigned int_.
Но при этом -25(_int_) не может быть сконвертировано в _unsigned int_, данная попытка приведет к ошибке компиляции

```
error: constant expression evaluates to -25 which cannot be narrowed to type 'unsigned int' [-Wc++11-narrowing]
```

[[implicit conversion]]
[[conversion in arithmetic operations]]
[[safety and unsafety conversions]]
[[explicit conversion]]

---
[Статическая типизация и преобразования типов|metanit.com](https://metanit.com/cpp/tutorial/2.4.php)