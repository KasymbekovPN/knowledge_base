---
tags:
  - programming-language
  - cpp
  - syntax
  - cycle
  - for-each
---
[[__cpp syntax construction cycle__|<==]]

Существует также особая форма цикла _for_, которая предназначена специально для работы с последовательностями значений.

```
for (type var: sequense) {
	instruction;
}
```

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    for (int n : {1, 2, 3}) {
        cout << "n <= " << n << endl;
    }

    for (char ch: "Hello") {
        cout << "ch <= '" << ch << "'" << endl;
    }

    return 0;
}
```

```
n <= 1
n <= 2
n <= 3
ch <= 'H'
ch <= 'e'
ch <= 'l'
ch <= 'l'
ch <= 'o'
ch <= ''
```

Выражение `{1, 2, 3}` как раз представляет последовательность значений - чисел _int_. Цикл перебирает всю эту последовательность и помещает каждое значение в переменную _n_.

Строка тоже последовательность. Каждый символ строки помещается в переменную _ch_.

---
[Циклы](https://metanit.com/cpp/tutorial/2.13.php)