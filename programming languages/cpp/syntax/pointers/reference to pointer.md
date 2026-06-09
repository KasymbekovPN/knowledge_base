---
tags:
  - programming-language
  - cpp
  - syntax
  - pointer
---
[[__cpp syntax pointers__|<==]]

Так как ссылка не является объектом, то нельзя определить указатель на ссылку, однако можно определить ссылку на указатель. Через подобную ссылку можно изменять значение, на которое указывает указатель или изменять адрес самого указателя.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int first {1};
    int second {42};
    int *pointer {};
    int *&pref {pointer};

    pref = &first;
    cout << "*pointer <= " << *pointer << endl;

    *pref = 111;
    cout << "first <= " << first << endl;

    pref = &second;
    cout << "*pointer <= " << *pointer << endl;

    return 0;
}
```

```
*pointer <= 1
first <= 111
*pointer <= 42
```

---
[Указатели](https://metanit.com/cpp/tutorial/4.1.php)