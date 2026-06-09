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

int main() {
	std::unordered_map<std::string, int> big_map;
	big_map.reserve(1000);

    return 0;
}
```
