---
tags:
  - programming-language
  - cpp
  - containers
---
[[_cpp containers span - access|<=]]

Метод `subspan()` возвращает новый `span` начиная с `offset` длиной в `len`

```cpp
subspan(const size_type offset, const size_type len)
```

```cpp
#include <iostream>
#include <span>

template <typename T>
void _print_span(const std::span<T>&);

int main(int argc, char const *argv[]) {
    int array[] {1, 2, 3, 4, 5};
    std::span<int> original {array};
    _print_span(original);

    auto sub_span = original.subspan(2, 3);
    _print_span(sub_span);
  
    return 0;
}

template <typename T>
void _print_span(const std::span<T>& s) {
    std::cout << "{";
    std::string delimiter = "";
    for (int &item: s) {
        std::cout << delimiter << item;
        delimiter = ", ";
    }
    std::cout << "}" << std::endl;
}
```

```
{1, 2, 3, 4, 5}
{3, 4, 5}
```
