---
tags:
  - programming-language
  - cpp
  - syntax
  - pointer
---
[[__cpp syntax pointers__|<==]]

К указателям могут применяться операции сравнения:
- `>`
- `=>`
- `==`
- `!=`
- `<`
- `<=`

 Операции сравнения применяются только к указателям одного типа. Для сравнения используются номера адресов.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int number0 {42};
    int number1 {1};

    int *pnumber0 {&number0};
    int *pnumber1 {&number1};

    bool comparison_result = pnumber0 > pnumber1;
    if (comparison_result) {
        cout << "(" << pnumber0 << ") > " << "(" << pnumber1 << ")";
    } else {
        cout << "(" << pnumber0 << ") <= " << "(" << pnumber1 << ")";
    }

    return 0;
}
```

```
(0000000ABE8FFC50) > (0000000ABE8FFC4C)
```

---
[Операции с указателями](https://metanit.com/cpp/tutorial/4.2.php)