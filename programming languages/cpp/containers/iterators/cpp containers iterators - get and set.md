---
tags:
  - programming-language
  - cpp
  - container
  - iterator
---
[[_cpp containers iterators|<=]]

Поскольку итератор по сути представляет указатель на определенный элемент, то через этот указатель мы можем получить текущий элемент итератора и изменить его значение.

```cpp
#include <iostream>
#include <vector>

int main(int argc, char const *argv[]) {
    std::vector<int> numbers {1, 2, 3};

    auto it {numbers.begin()};
    std::cout << "numbers[0] <= " << *it << std::endl;

    *it = 42;
    std::cout << "numbers[0] <= " << *it << std::endl;

    it += 2;
    std::cout << "numbers[2] <= " << *it << std::endl;

    return 0;
}
```

```
numbers[0] <= 1
numbers[0] <= 42
numbers[2] <= 3
```

---
[Итераторы](https://metanit.com/cpp/tutorial/7.3.php)