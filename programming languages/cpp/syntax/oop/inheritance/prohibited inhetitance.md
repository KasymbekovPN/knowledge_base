---
tags:
  - programming-language
  - cpp
  - syntax
  - oop
  - class
  - final
---
[[__cpp syntax oop inheritance__|<==]]

В случает, если наследование класса нежелательно, то при помощи ключевого слова _final_ его можно запретить.

```cpp
#include <iostream>

class Person final {};


class Employee: public Person {};


int main(int argc, char const *argv[]) {
    Employee tom {};

    return 0;
}
```

```
.\prohibited.cpp:6:24: error: base 'Person' is marked 'final'
    6 | class Employee: public Person {};
      |                        ^
.\prohibited.cpp:3:7: note: 'Person' declared here
    3 | class Person final {};
      |       ^      ~~~~~
```

---
[Наследование](https://metanit.com/cpp/tutorial/5.10.php)