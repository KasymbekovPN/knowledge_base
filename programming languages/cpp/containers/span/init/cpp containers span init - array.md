---
tags:
  - programming-language
  - cpp
  - containers
---
[[_cpp containers span - init|<=]]

```cpp
#include <iostream>
#include <array>
#include <span>

template <typename T>
void _print_span(const std::span<T>&);

int main(int argc, char const *argv[]) {
    std::array<int, 4> arr {1, 2, 3, 4};
    std::span<int> s {arr};
    _print_span(s);

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
{1, 2, 3, 4}
```
