---
tags:
  - programming-language
  - cpp
  - containers
  - map
---
[[_cpp containers unordered_map - init|<=]]

```cpp
#include <iostream>
#include <unordered_map>
#include <vector>

void _print_map(const std::unordered_map<int, std::string>&);

int main() {
    std::vector<std::pair<int, std::string>> source {
        {1, "one"},
        {2, "two"},
        {3, "three"},
        {4, "four"}
    };

    auto begin_it = source.begin();
    auto end_it = source.begin();
    std::advance(begin_it, 1);
    std::advance(end_it, 3);

    std::unordered_map<int, std::string> map {begin_it, end_it};
    _print_map(map);

    return 0;
}

void _print_map(const std::unordered_map<int, std::string>& uomap) {
    for (auto &[key, value]: uomap) {
        std::cout
            << "{ " << key
            << ", " << value
            << " }" << std::endl;
    }
}
```

```
{ 2, two }
{ 3, three }
```
