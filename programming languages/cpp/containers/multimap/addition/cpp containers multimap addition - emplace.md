---
tags:
  - programming-language
  - cpp
  - container
  - multimap
---
[[_cpp containers multimap - addition|<=]]

Методы `emplace` предоставляют эффективный способ добавления элементов в `std::multimap` путем конструирования их на месте (in-place construction), что позволяет избежать лишних копирований и перемещений.

```cpp
#include <iostream>
#include <map>
#include <vector>

void _print_mmap(const std::multimap<int, std::string>&);
void _print_new(const std::multimap<int, std::string>::iterator&);

int main() {
    std::multimap<int, std::string> mmap;
    _print_new(mmap.emplace(1, "one"));
    _print_new(mmap.emplace(1, "one one"));
    _print_new(mmap.emplace(2, "two"));

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

void _print_new(const std::multimap<int, std::string>::iterator& it) {
    std::cout
        << "NEW {kye: " << it->first
        << ", value: " << it->second
        << "}" << std::endl;
}
```

```
NEW {kye: 1, value: one}
NEW {kye: 1, value: one one}
NEW {kye: 2, value: two}
{key: 1, value: one}
{key: 1, value: one one}
{key: 2, value: two}
```
