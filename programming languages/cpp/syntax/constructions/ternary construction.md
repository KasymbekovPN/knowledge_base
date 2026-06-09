---
tags:
  - programming-language
  - cpp
  - syntax
  - construction
  - ternary
---
[[__cpp syntax constractions__|<=]]

Тернарный оператор в некотором роде похож на конструкцию _if_-_else_. Он принимает три операнда в следующем виде:

```
operand0? operand1: operand2;
```

Если это _operand0_ == _true_, тогда выбирается/выполняется _operand1_, который помещается после символа _?_. Если условие не верно, тогда выбирается/выполняется _operand2_, который помещается после _:_.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int a{5};
    int b{3};

    if (a > b) {
        cout << "[if] a > b" << endl;
    } else {
        cout << "[if] a <= b" << endl;
    }

    cout << (a > b ? "[T] a > b" : "[T] a <= b") << endl;

    return 0;
}
```

Тернарный оператор не обязательно должен возвращать некоторое значение, он может просто выполнять некоторые действия.

---
[Конструкции](https://metanit.com/cpp/tutorial/2.12.php)