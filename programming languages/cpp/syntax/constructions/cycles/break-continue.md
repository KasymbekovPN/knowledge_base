---
tags:
  - programming-language
  - cpp
  - syntax
  - cycle
  - break
  - continue
---
[[__cpp syntax construction cycle__|<==]]

Иногда возникает необходимость выйти из цикла до его завершения, либо пропустить итерацию и перейти к следующей. Для первого используется оператор _break_, для второго - _continue_.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    for (int i{}; i < 20; i++) {
        if (i % 2 == 0) {
            continue;
        }

        if (i > 15) {
            break;
        }

        cout << "i <= " << i << endl;
    }

    return 0;
}
```

```
i <= 1
i <= 3
i <= 5
i <= 7
i <= 9
i <= 11
i <= 13
i <= 15
```

---
[Циклы](https://metanit.com/cpp/tutorial/2.13.php)