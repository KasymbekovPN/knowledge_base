---
tags:
  - programming-language
  - cpp
  - function
  - return
  - data-types
  - syntax
---
[[__cpp syntax functions__|<==]]

При определении указателя на функцию применяется синтаксис, который может показаться плохо читаемым.

```cpp
int (*operation)(int, int) {sum};
```

 Использование ключевого слова _auto_ позволяет упростить определение.

```cpp
auto operation {sum};
```

Однако в данном случае нам надо обязательно инициализировать указатель определенной функцией. Кроме того, иногда возникает необходимость явным образом указать тип указателя, например, для параметра функции или переменной. Так как указатель на функцию представляет отдельный тип, то для него можно определить псевдоним с помощью ключевого слова _using_.

```cpp
#include <iostream>

using BinaryOp = int (*)(int, int);

int sum(int, int);
int sub(int, int);
int do_operation(BinaryOp, int, int);

int main(int argc, char const *argv[]) {
    int first {6};
    int second {9};

    std::cout
	    << "Sum of " << first << " & " << second
	    << " => " << do_operation(sum, first, second) << std::endl;
    std::cout
	    << "Sub of " << first << " & " << second
	    << " => " << do_operation(sub, first, second) << std::endl;

    return 0;
}

int sum(int first, int second) {
    return first + second;
}

int sub(int first, int second) {
    return first - second;
}

int do_operation(BinaryOp op, int first, int second) {
    return op(first, second);
}
```

```
Sum of 6 & 9 => 15
Sub of 6 & 9 => -3
```

---
[Тип функции](https://metanit.com/cpp/tutorial/4.15.php)