---
tags:
  - programming-language
  - cpp
  - syntax
  - function
---

Когда запускается программа на языке __C++__, то запускается функция _main_. Никакие другие функции, определенные в программе, автоматически не выполняются. Для выполнения функции ее необходимо вызвать. Вызов функции осуществляется в форме.

```
function_name(arguments);
```

После имени функции указываются скобки, в которых перечисляются аргументы - значения для параметров функции.

```cpp
#include <iostream>

using std::cout;
using std::endl;

void hello() {
    cout << "hello" << endl;
}

int main(int argc, char const *argv[]) {
    hello();
    hello();

    return 0;
}
```

```
hello
hello
```

---
[Определение и объявление функции](https://metanit.com/cpp/tutorial/3.1.php)