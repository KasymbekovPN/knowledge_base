---
tags:
  - programming-language
  - cpp
  - container
  - multimap
---
[[_cpp containers multimap - init|<=]]

Инициализация список доступна с __C++11__.

```cpp
#include <iostream>
#include <map>

void _print_mmap(const std::multimap<int, std::string>&);

int main() {
    std::multimap<int, std::string> mmap {
        {1, "one"},
        {2, "two"},
        {1, "other one"}
    };
    _print_mmap(mmap);

    return 0;
}

void _print_mmap(const std::multimap<int, std::string>& mmap) {
    for (auto &[key, value]: mmap) {
        std::cout
            << "{key: " << key
            << ", value: " << value
            << "}" << std::endl;
    }
}
```

```
{key: 1, value: one}
{key: 1, value: other one}
{key: 2, value: two}
```
