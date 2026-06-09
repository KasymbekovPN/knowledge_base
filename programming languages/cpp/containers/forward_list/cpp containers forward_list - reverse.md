---
tags:
  - programming-language
  - cpp
  - syntax
  - container
  - forward_list
---
[[_cpp containers - forward_list|<=]]

Метод _reverse()_ переворачивает порядок элементов в списке.

```cpp
#include <iostream>
#include <forward_list>

template<typename T>
void print_flist(const std::forward_list<T>&);

int main() {
    std::forward_list<int> numbers {123, 17, 200, 1, -1};
    print_flist(numbers);
  
    numbers.reverse();
    print_flist(numbers);

    return 0;
}

template<typename T>
void print_flist(const std::forward_list<T>& flist) {
    std::cout << "flist => ";
    for (const auto &item: flist) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}
```

```
flist => 123 17 200 1 -1 
flist => -1 1 200 17 123
```
