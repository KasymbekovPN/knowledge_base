---
tags:
  - programming-language
  - cpp
  - container
  - multimap
---
[[_cpp containers multimap - init|<=]]

```cpp
#include <iostream>
#include <map>
#include <vector>

void _print_mmap(const std::multimap<int, std::string>&);

int main() {
    std::vector<std::pair<int, std::string>> source = {
        {1, "one"},
        {1, "one one"},
        {2, "two"}
    };

    std::multimap<int, std::string> mmap {source.begin(), source.end()};
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
{key: 1, value: one one}
{key: 2, value: two}
```