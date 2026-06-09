---
tags:
  - programming-language
  - cpp
  - syntax
  - pointer
---
[[__cpp syntax pointers__|<==]]

По адресу можно получить хранящееся там значение, то есть значение переменной _number_. Для этого применяется операция _*_ или операция разыменования (__indirection operator__/ __dereference operator__). Результатом этой операции всегда является объект, на который указывает указатель. Применим данную операцию и получим значение переменной _number_.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int number {42};
    int* pnumber {&number};
    int number_copy {*pnumber};

    cout << "number <= " << number << endl;
    cout << "pnumber <= " << *pnumber << endl;
    cout << "number_copy <= " << number_copy << endl;

    *pnumber = 24;
    cout << "number <= " << number << endl;
    cout << "pnumber <= " << *pnumber << endl;
    cout << "number_copy <= " << number_copy << endl;

    return 0;
}
```

```
number <= 42
pnumber <= 42
number_copy <= 42
number <= 24
pnumber <= 24
number_copy <= 42
```

И также используя указатель, мы можем менять значение по адресу, который хранится в указателе.

---
[Указатели](https://metanit.com/cpp/tutorial/4.1.php)