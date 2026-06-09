---
tags:
  - programming-language
  - cpp
  - syntax
  - container
  - list
---
[[_cpp containers list|<=]]

Метод _push_front()_ добавляет элемент в начало списка.

```cpp
#include <iostream>
#include <list>

template<typename T>
void print_list(const std::list<T>&);

int main() {
    std::list<int> numbers {1, 2, 3};
    print_list(numbers);

    numbers.push_front(42);
    print_list(numbers);

    return 0;
}

template<typename T>
void print_list(const std::list<T>& list) {
    for (auto &&item: list) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}
```

```
1 2 3 
42 1 2 3
```