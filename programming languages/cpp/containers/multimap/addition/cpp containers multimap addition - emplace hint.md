---
tags:
  - programming-language
  - cpp
  - container
  - multimap
---
[[_cpp containers multimap - addition|<=]]

Методы `emplace` предоставляют эффективный способ добавления элементов в `std::multimap` путем конструирования их на месте (in-place construction), что позволяет избежать лишних копирований и перемещений.

- В лучшем случае O(1) (если подсказка точная). 
- В худшем случае O(log n) (если подсказка неверная).

```cpp
#include <iostream>
#include <map>
#include <vector>

void _print_mmap(const std::multimap<int, std::string>&);

int main() {
    std::multimap<int, std::string> mmap = {
        {1, "one"},
        {3, "three"},
        {5, "five"},
    };

    auto hint = mmap.find(3);
    if (hint != mmap.end()) {
        mmap.insert(hint, {7, "seven"});
    }

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
{key: 3, value: three}
{key: 5, value: five}
{key: 7, value: seven}
```
