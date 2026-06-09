---
tags:
  - programming-language
  - cpp
  - syntax
  - pointer
---
[[__cpp syntax pointers__|<==]]

Нулевой указатель (_null pointer_) - это указатель, который не указывает ни на какой объект. Если мы не хотим, чтобы указатель указывал на какой-то конкретный адрес, то можно присвоить ему условное нулевое значение. Для определения нулевого указателя можно инициализировать указатель нулем или константой _nullptr_.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int* p0 {};
    int* p1 {nullptr};

    cout << "p0 <= " << p0 << endl;
    cout << "p1 <= " << p1 << endl;

    return 0;
}
```

```
p0 <= 0000000000000000
p1 <= 0000000000000000
```

---
[Указатели](https://metanit.com/cpp/tutorial/4.1.php)