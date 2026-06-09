---
tags:
  - programming-language
  - cpp
  - syntax
  - operation
  - shift
---
[[__cpp syntax operations__|<=]]

Каждое целое число в памяти представлено в виде определенного количества разрядов. И операции сдвига позволяют сдвинуть битовое представление числа на несколько разрядов вправо или влево. Операции сдвига применяются только к целочисленным операндам. Есть две операции:

- `<<` - cдвигает битовое представление числа, представленного первым операндом, влево на определенное количество разрядов, которое задается вторым операндом.
- `>>` - cдвигает битовое представление числа вправо на определенное количество разрядов.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    unsigned int offset {3};
    unsigned int original {7};
    unsigned int result {original << offset};
    cout << original << " << " << offset << " = " << result << endl;

    original = 26;
    result = original >> offset;
    cout << original << " >> " << offset << " = " << result << endl;

    return 0;
}
```

```
7 << 3 = 56
26 >> 3 = 3
```

---
[[double representation of numbers]]
[[representation of negative numbers]]
[Поразрядные операции](https://metanit.com/cpp/tutorial/2.8.php)