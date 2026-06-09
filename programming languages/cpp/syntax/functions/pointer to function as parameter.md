---
tags:
  - programming-language
  - cpp
  - syntax
  - function
  - pointer
---
[[__cpp syntax functions__|<==]]

Указатель на функцию фактически представляет некоторый тип, и функция также может иметь параметр, который представляет тип указателя на функцию. Таким образом, мы можем через параметр на функцию передавать в одну функцию другую. То есть функция может быть аргументом другой функции.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int add(int, int);
int sub(int, int);
int operation(int (*)(int, int), int, int);

int main(int argc, char const *argv[]) {
    const int x {42};
    const int y {73};

    cout << "ADD result <= " << operation(add, x, y) << endl;
    cout << "SUB result <= " << operation(sub, x, y) << endl;

    return 0;
}

int add(int x, int y) {
    return x + y;
}

int sub(int x, int y) {
    return x - y;
}

int operation(int (*op)(int, int), int x, int y) {
    return op(x, y);
}
```

```
ADD result <= 115
SUB result <= -31
```

---
[Указатели на функции как параметры](https://metanit.com/cpp/tutorial/4.9.php)