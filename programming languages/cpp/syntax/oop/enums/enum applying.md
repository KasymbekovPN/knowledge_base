---
tags:
  - programming-language
  - cpp
  - syntax
  - enum
---
[[__cpp syntax oop enums__|<==]]

Перечисления удобны, когда необходимо хранить ограниченный набор состояний и в зависимости от текущего состояния выполнять некоторые действия. 

В данном случае все арифметические операции хранятся в перечислении _Operation_. В функции _calculate_ зависимости от значения третьего параметра - применяемой операции выполняются определенные действия с двумя первыми параметрами.

```cpp
#include <iostream>

enum class Operation {
    add,
    sub,
    mul
};

void execute(int, int, Operation);

int main(int argc, char const *argv[]) {
    execute(10, 15, Operation::add);
    execute(10, 15, Operation::sub);
    execute(10, 15, Operation::mul);

    return 0;
}

void execute(int i0, int i1, Operation op) {
    switch (op) {
        case Operation::add:
            std::cout
	            << i0 << " + " << i1 << " <= " << i0 + i1 << std::endl;
            break;

        case Operation::sub:
            std::cout
	            << i0 << " - " << i1 << " <= " << i0 - i1 << std::endl;
            break;

        case Operation::mul:
            std::cout
	            << i0 << " * " << i1 << " <= " << i0 * i1 << std::endl;
            break;
    }
}
```

```
10 + 15 <= 25
10 - 15 <= -5
10 * 15 <= 150
```

---
[Перечисления](https://metanit.com/cpp/tutorial/5.9.php)