---
tags:
  - programming-language
  - cpp
  - syntax
  - construction
  - cycle
  - for
---
[[__cpp syntax construction cycle__|<==]]

```
for (initializator; condition; iteration) {
	instruction;
}
```

 - __initilizaror__ выполняется один раз при начале выполнения цикла и представляет установку начальных условий.
 - __condition__ представляет условие, при соблюдении которого выполняется цикл. 
 - __iteration__ выполняется после каждого завершения блока цикла и задает изменение параметров цикла.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    for (int i{}; i < 3; i++) {
        cout << "i <= " << i << endl;
    }

    return 0;
}
```

```
i <= 0
i <= 1
i <= 2
```

- `int i{}` - устанавливает счетчик _i_ в ноль.
- `i < 3` - условие выполнения цикла.
- `i++` - инструкция, выполняющаяся после каждой итерации.

---
[Циклы](https://metanit.com/cpp/tutorial/2.13.php)