---
tags:
  - programming-language
  - cpp
  - syntax
  - include
---
[[__cpp syntax libraries__|<==]]

Здесь файлы main.cpp, sum.h, sum.cpp в одной директории.

__sum.h__
```cpp
int sum(int, int);
```

__sum.cpp__
``` cpp
int sum(int first, int second) {
    return first + second;
}
```

__main.cpp__
```cpp
#include <iostream>
#include "sum.h"

int main(int argc, char const *argv[]) {
    int first {5};
    int second {12};

    std::cout
	    << "Sum of " << first << " & " << second
	    << " => " << sum(first, second) << std::endl;

    return 0;
}
```

Функция _main_ вызывает функцию _sum_ для вычисления суммы чисел. Но перед использованием функции она должна быть определена или по крайней мере должен быть известен ее заголовок.  Модуль с функцией подключается при помощи _include_.

Скомпилировать:
```
clang++.exe .\main.cpp .\sum.cpp -o .\main.exe
```

Результат:
```
Sum of 5 & 12 => 17
```

---
[Разделение программы на файлы](https://metanit.com/cpp/tutorial/3.8.php)