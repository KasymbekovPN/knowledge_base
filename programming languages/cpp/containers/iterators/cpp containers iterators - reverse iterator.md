---
tags:
  - programming-language
  - cpp
  - container
  - iterator
  - reverse
---
[[_cpp containers iterators|<=]]

Реверсивные итераторы позволяют перебирать элементы контейнера в обратном направлении. Для получения реверсивного итератора применяются функции _rbegin()_ и _rend()_, а сам итератор представляет тип _reverse_iterator_ (`std::vector<int>::reverse_iterator`).

Если надо обеспечить защиту от изменения значений контейнера, то можно использовать константный реверсивный итератор, который представлен типом _const_reverse_iterator_ и который можно получить с помощью функций _crbegin()_ и _crend()_.

```cpp
#include <iostream>
#include <vector>

int main(int argc, char const *argv[]) {
    std::vector<int> numbers {1, 2, 3};
    for (auto it {numbers.rbegin()}; it != numbers.rend(); it++) {
        std::cout << *it << "\t";
    }
    std::cout << std::endl;

    for (auto it {numbers.crbegin()}; it != numbers.crend(); it++) {
        // *it = (*it) + 1; //< Error
        std::cout << *it << "\t";
    }

    return 0;
}
```

```
3       2       1
3       2       1
```

```
.\reverse_iterator.cpp:12:13: error: cannot assign to return value because function 'operator*' returns a const value
   12 |         *it = (*it) + 1; //< Error
      |         ~~~ ^
```

---
[Итераторы](https://metanit.com/cpp/tutorial/7.3.php)