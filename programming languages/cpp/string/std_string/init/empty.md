---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/std_string/init/_|<=]]

```cpp
#include <iostream>
#include <string>

int main() {
    std::string line;
    std::cout
        << "'" << line << "' size: "
        << line.size() << std::endl;

    return 0;
}
```

```
'' size: 0
```
