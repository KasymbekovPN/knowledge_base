---
tags:
  - programming-language
  - cpp
  - syntax
  - container
  - list
---
[[_cpp containers list|<=]]

Метод _remove()_`удаляет все элементы с указанным значением.

```cpp
#include <iostream>
#include <list>

template<typename T>
void print_list(const std::list<T>&);

int main() {
    std::list<int> numbers {1, 2, 1, 4, 1, 6, 1};
    print_list(numbers);

    numbers.remove(1);
    print_list(numbers);

    return 0;
}

template<typename T>
void print_list(const std::list<T>& list) {
    for (const auto &item: list) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}
```

```
1 2 1 4 1 6 1 
2 4 6
```