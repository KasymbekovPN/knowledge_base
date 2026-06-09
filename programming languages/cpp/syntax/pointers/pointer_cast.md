---
tags:
  - programming-language
  - cpp
  - syntax
  - pointer
---
[[__cpp syntax pointers__|<==]]

Иногда требуется присвоить указателю одного типа значение указателя другого типа. В этом случае следует выполнить операцию приведения типов с помощью операции `(pointer_type *)`:

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    char ch {'x'};
    char* pch {&ch};
    int* pnumber {(int*)pch};
    void* pvoid {(void*)pch};

	cout << "pch: " << pch << endl;
    cout << "pnumber: " << pnumber << " | *pnumber: " << *pnumber << endl;
    cout << "pvoid: "   << pvoid   << endl;

    return 0;
}
```

```
pch: x☺
pnumber: 0000007811EFFCB3 | *pnumber: 376
pvoid: 0000007811EFFCB3
```

Для преобразования указателя к другому типу в скобках перед указателем ставится тип, к которому надо преобразовать. Причем если мы не можем просто создать объект, например, переменную типа _void_, то для указателя это вполне будет работать. То есть можно создать указатель типа _void_.

Кроме того, следует отметить, что указатель на тип _char_ `char* pch {&ch};` при выводе на консоль система интерпретирует как строку.

Поэтому если мы все-таки хотим вывести на консоль адрес, который хранится в указателе типа _char_, то это указатель надо преобразовать к другому типу, например, к _void*_ или к _int*_.

---
[Операции с указателями](https://metanit.com/cpp/tutorial/4.2.php)