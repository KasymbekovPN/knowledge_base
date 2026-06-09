---
tags:
  - programming-language
  - cpp
  - syntax
  - pointer
---
[[__cpp syntax pointers__|<==]]

Указателю можно присвоить адрес объекта того же типа, либо значение другого указателя. Для получения адреса объекта используется операция &

При этом указатель и переменная должны иметь один и тот же тип, в данном случае это тип _int_.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int first {1};
    int second {42};

    int* poiner_first {&first};
    int* pointer_second {&second};

    cout << "*pointer_first <= " << *poiner_first << endl;
    cout << "*pointer_second <= " << *pointer_second << endl;

    pointer_second = poiner_first;
    cout << "*pointer_first <= " << *poiner_first << endl;
    cout << "*pointer_second <= " << *pointer_second << endl;

    return 0;
}
```

```
*pointer_first <= 1
*pointer_second <= 42
*pointer_first <= 1
*pointer_second <= 1
```

---
[Указатели](https://metanit.com/cpp/tutorial/4.1.php)