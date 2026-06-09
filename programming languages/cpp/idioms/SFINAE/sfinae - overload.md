---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/SFINAE/_|<=]]

```cpp
#include <iostream>
#include <vector>

template<typename T>
auto print_size(const T&& _value) -> decltype(_value.size(), void ()) {
    std::cout
        << "Iterable size: "
        << _value.size()
        << std::endl;
}

void print_size(...) {
    std::cout
        << "Unknown size"
        << std::endl;
}

int main() {
    print_size(42);
    print_size(std::vector<int>{1, 2, 3, 4, 5});

    return 0;
}
```

```
Unknown size
Iterable size: 5
```
