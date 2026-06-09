---
tags:
  - programming-language
  - cpp
  - syntax
  - construction
  - cycle
  - do-while
---
[[__cpp syntax construction cycle__|<==]]

В цикле _do_ сначала выполняется код цикла, а потом происходит проверка условия в инструкции _while_. И пока это условие истинно цикл повторяется.

```
do {
	instruction;
} while(condition);
```

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int i{5};

	do {
        cout << "i <= " << i-- << endl;
    } while (i > 0);

    return 0;
}
```

```
i <= 5
i <= 4
i <= 3
i <= 2
i <= 1
```

Здесь код цикла сработает __5__ раз, пока i не станет равным нулю.

Но важно отметить, что цикл do гарантирует хотя бы однократное выполнение действий, даже если условие в инструкции while не будет истинно.

---
[Циклы](https://metanit.com/cpp/tutorial/2.13.php)