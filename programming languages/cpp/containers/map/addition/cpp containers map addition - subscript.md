---
tags:
  - programming-language
  - cpp
  - container
  - map
---
[[_cpp containers map - addition|<=]]

```cpp
#include <iostream>
#include <map>
#include <string>

void _print_pair(
	const std::pair<std::map<std::string, int>::iterator, bool>&
);
void _print_map(const std::map<std::string, int>&);

int main() {
    std::map<std::string, int> map;
    map["one"] = 1;
    map["two"] = 2;

    _print_map(map);

    return 0;
}

void _print_map(const std::map<std::string, int>& m) {
    for (auto &pair: m) {
        std::cout << "{" << pair.first
            << ", " << pair.second
            << "}" << std::endl;
    }
}

void _print_pair(
	const std::pair<std::map<std::string, int>::iterator, bool>& pair
) {
    std::cout
        << "{" << (*pair.first).first
        << ", " << (*pair.first).second
        << std::boolalpha
        << ", " << pair.second
        << std::noboolalpha
        << "}" << std::endl;
}
```

```
{one, 1}
{two, 2}
```