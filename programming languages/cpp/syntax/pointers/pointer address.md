---
tags:
  - programming-language
  - cpp
  - syntax
  - pointer
---
[[__cpp syntax pointers__|<==]]

Указатель хранит адрес переменной, и по этому адресу мы можем получить значение этой переменной. Но кроме того, указатель, как и любая переменная, сам имеет адрес, по которому он располагается в памяти. Этот адрес можно получить также через операцию _&_.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int number {42};
    int *pointer {&number};

    cout << "pointer <= " << pointer << endl;

    return 0;
}
```

```
pointer <= 0000006A8A2FF990
```

---
[Операции с указателями](https://metanit.com/cpp/tutorial/4.2.php)