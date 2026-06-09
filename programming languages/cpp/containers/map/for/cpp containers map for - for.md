---
tags:
  - programming-language
  - cpp
  - container
  - map
---
[[_cpp containers map - for|<=]]

```cpp
#include <iostream>
#include <map>

int main() {
    const std::map<std::string, int> map {
        {"one", 1},
        {"two", 2}
    };

    for (auto &i: map) {
        std::cout
            << "[" << i.first
            << ", " << i.second
            << "]" << std::endl;
    }

    for (auto &[key, value]: map) {
        std::cout
            << "[" << key
            << ", " << value
            << "]" << std::endl;
    }

    return 0;
}
```

```
[one, 1]
[two, 2]
[one, 1]
[two, 2]
```


---
#### 6. Обход элементов
```cpp
for (const auto& [key, value] : m) {
    std::cout << key << ": " << value << std::endl;
}
```

#### 7. Границы диапазонов
```cpp
auto lb = m.lower_bound("b");  // Первый элемент >= "b"
auto ub = m.upper_bound("c");  // Первый элемент > "c"
```
