---
tags:
  - programming-language
  - cpp
  - syntax
  - pointer
---
[[__cpp syntax pointers__|<==]]

Операция разыменования указателя представляет выражение в виде `*pointer_name`. Эта операция позволяет получить объект по адресу, который хранится в указателе.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int number {42};
    int* pnumber {&number};

    cout << "number <= " << number
         << " | *number <= " << *pnumber
         << " | number <= " << pnumber << endl;

    *pnumber = 12;
    cout << "number <= " << number
         << " | *number <= " << *pnumber
         << " | number <= " << pnumber << endl;

    return 0;
}
```

Через выражение `*pnumber` мы можем получить значение по адресу, который хранится в указателе `pnumber`, а через выражение типа `*pnumber = ...` вложить по этому адресу новое значение.

---
[Указатели](https://metanit.com/cpp/tutorial/4.1.php)