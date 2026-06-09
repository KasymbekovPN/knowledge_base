[[__cpp syntax pointers__|<==]]

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int first {42};
    int second {1};
    int* p0 {&first};
    int* p1 {&second};

    cout << "*p0 <= " << *p0 << "\t| p0 <= " << p0 << endl;
    cout << "*p1 <= " << *p1 << "\t| p1 <= " << p1 << endl;

    p0 = p1;
    *p0 = 123;
    cout << "*p0 <= " << *p0 << "\t| p0 <= " << p0 << endl;
    cout << "*p1 <= " << *p1 << "\t| p1 <= " << p1 << endl;

    return 0;
}
```

```
*p0 <= 42       | p0 <= 000000F41E6FFC00
*p1 <= 1        | p1 <= 000000F41E6FFBFC
*p0 <= 123      | p0 <= 000000F41E6FFBFC
*p1 <= 123      | p1 <= 000000F41E6FFBFC
```

Когда указателю присваивается другой указатель, то фактически первый указатель начинает также указывать на тот же адрес, на который указывает второй указатель.

---
[Указатели](https://metanit.com/cpp/tutorial/4.1.php)