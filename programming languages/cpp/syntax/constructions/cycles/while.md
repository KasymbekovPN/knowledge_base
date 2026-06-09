---
tags:
  - programming-language
  - cpp
  - syntax
  - construction
  - cycle
  - while
---
[[__cpp syntax construction cycle__|<==]]

Цикл _while_ выполняет некоторый код, пока его условие истинно. Он имеет следующее формальное определение:

```
while (condition) {
	instruction;
}
```

После ключевого слова _while_ в скобках идет условное выражение, которое возвращает _true_ или _false_. Затем в фигурных скобках идет набор инструкций, которые составляют тело цикла. И пока условие возвращает _true_, будут выполняться инструкции в теле цикла.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int i{};

    while (++i <= 3) {
        cout << "i <= " << i << endl;
    }
    cout << "Done" << endl;

    return 0;
}
```

```
i <= 1
i <= 2
i <= 3
Done
```

Здесь пока условие `i <= 3` истинно, будет выполняться цикл _while_. В какой-то момент переменная _i_ увеличится до __4__, условие перестает быть истинным, и цикл завершится.

---
[Циклы](https://metanit.com/cpp/tutorial/2.13.php)