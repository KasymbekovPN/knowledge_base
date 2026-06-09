---
tags:
  - programming-language
  - cpp
  - syntax
  - pointer
  - constants
---
[[__cpp syntax pointers__|<==]]

#### Указатели на константы

Указатели могут указывать как на переменные, так и на константы. Чтобы определить указатель на константу, он тоже должен объявляться с ключевым словом _const_.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    const int number {42};
    const int* pnumber {&number};

    cout << "addr <= " << pnumber << " | value <= " << *pnumber << endl;

    return 0;
}
```

```
addr <= 0000009663EFFAF0 | value <= 42
```

Т.к. _pnumber_ указывает на константу мы не может изменить значение константы, но может изменить адрес, записанный в указатель.

Возможна также ситуация, когда указатель на константу на самом деле указывает на переменную. В этом случае переменную отдельно мы сможем изменять, однако по прежнему изменить ее значение через указатель мы не сможем.

#### Константный указатель

От указателей на константы надо отличать константные указатели. Они не могут изменять адрес, который в них хранится, но могут изменять значение по этому адресу.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int number {42};
    int other {1};
    int* const pnumber {&number};
    cout << "*pnumber <= " << *pnumber << endl;

    (*pnumber)++;
    cout << "*pnumber <= " << *pnumber << endl;

    // pnumber = &other;

    return 0;
}
```

```
*pnumber <= 42
*pnumber <= 43
```

```
.\const_pointer.cpp:16:13: error: cannot assign to variable 'pnumber' with const-qualified type 'int *const'
   16 |     pnumber = &other;
      |     ~~~~~~~ ^
```

#### Константный указатель на константу

И объединение обоих предыдущих случаев - константный указатель на константу, который не позволяет менять ни хранимый в нем адрес, ни значение по этому адресу.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int other {1};
    int number {42};
    const int* const pnumber {&number};
    cout << "pnumber <= " << *pnumber << endl;

    // *pnumber = 123; // Error
    // pnumber = &other; // Error

    return 0;
}
```

```
pnumber <= 42
```

```
.\const_pointer2const.cpp:12:14: error: read-only variable is not assignable
   12 |     *pnumber = 123;
      |     ~~~~~~~~ ^
```

```
.\const_pointer2const.cpp:13:13: error: cannot assign to variable 'pnumber' with const-qualified type 'const int *const'
   13 |     pnumber = &other;
      |     ~~~~~~~ ^
```

---
https://metanit.com/cpp/tutorial/4.4.php