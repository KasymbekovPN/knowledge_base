---
tags:
  - programming-language
  - cpp
  - container
  - iterator
  - constants
---
[[_cpp containers iterators|<=]]

Если контейнер представляет константу, то для обращения к элементам этого контейнера можно использовать только константный итератор (`std::vector<int>::const_iterator`). 

Для получения константного итератора также можно использовать функции _cbegin()_ и _cend()_. При этом даже если контейнер не представляет константу, но для его перебора используется константный итератор, то опять же нельзя изменять значения элементов этого контейнера.

Стоит отметить, что для типов std::set (множество) и std::map (словарь) доступны только константные итераторы.

```cpp
#include <iostream>
#include <vector>

int main(int argc, char const *argv[]) {
    const std::vector<int> NUMBERS {1, 2, 3};
    for (std::vector<int>::const_iterator it {NUMBERS.cbegin()}; it != NUMBERS.cend(); it++) {
        std::cout << "[#0] <= " << *it << std::endl;
        // *it = (*it) * (*it); // <= Error (0)
    }

    std::vector<int> numbers {1, 2, 3};
    for (std::vector<int>::const_iterator it {numbers.cbegin()}; it != numbers.cend(); it++) {

        std::cout << "[#1] <= " << *it << std::endl;
        // *it = (*it) * (*it); // <= Error (1)
    }

    return 0;
}
```
__Output:__
```
[#0] <= 1
[#0] <= 2
[#0] <= 3
[#1] <= 1
[#1] <= 2
[#1] <= 3
```

__Error (0)__
```
.\const_iterator.cpp:8:13: error: cannot assign to return value because function 'operator*' returns a const value
    8 |         *it = (*it) * (*it); // <= Error (0)
      |         ~~~ ^
```

__Error (1)__
```
.\const_iterator.cpp:14:13: error: cannot assign to return value because function 'operator*' returns a const value
   14 |         *it = (*it) * (*it); // <= Error (1)
      |         ~~~ ^
```

---
[Итераторы](https://metanit.com/cpp/tutorial/7.3.php)