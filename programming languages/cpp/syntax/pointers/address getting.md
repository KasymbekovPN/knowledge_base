---
tags:
  - programm
  - cpp
  - syntax
  - pointer
---
[[__cpp syntax pointers__|<==]]

С помощью операция _&_ можно получить адрес некоторого объекта, например, адрес переменной. Затем этот адрес можно присвоить указателю.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int number {42};
    int* pnumber {&number};

    cout << "  number <= " << number << endl;
    cout << " pnumber <= " << pnumber << endl;
    cout << "*pnumber <= " << *pnumber << endl;

    return 0;
}
```

```
  number <= 42
 pnumber <= 000000D396AFF730
*pnumber <= 42
```

Выражение `&number` возвращает адрес переменной `number`. Поэтому переменная `pnumber` будет хранить адрес переменной `number`.

В каждом отдельном случае адрес может отличаться и при разных запусках программы может меняться.

---
[Указатели](https://metanit.com/cpp/tutorial/4.1.php)