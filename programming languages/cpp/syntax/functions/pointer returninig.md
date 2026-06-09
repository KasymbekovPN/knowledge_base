---
tags:
  - programming-language
  - cpp
  - syntax
  - function
  - return
  - pointer
---
[[__cpp syntax functions__|<==]]

При возвращении указателя из функции он должен содержать либо значение _nullptr_, либо адрес, который все еще действителен. Поэтому не следует возвращать из функции адрес автоматической локальной переменной, так как она удаляется после завершения этой функции. 

```cpp
#include <iostream>

using std::cout;
using std::endl;

int* max(int*, int*);

int main(int argc, char const *argv[]) {
    int n{4};
    int m{5};

    int* ptr = max(&n, &m);
    cout << "max(" << n << ", " << m << ") <= " << *ptr  << endl;

    return 0;
}

int* max(int* n, int* m) {
    return *n > *m ? n : m;
}
```

```
max(4, 5) <= 5
```

---
[Возвращение указателя из функции](https://metanit.com/cpp/tutorial/4.13.php)