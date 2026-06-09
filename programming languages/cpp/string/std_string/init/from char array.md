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
    const char original[] = "Hello, world!!!";
    std::string line {original};
    std::cout
        << "'" << line << "' size: "
        << line.size() << std::endl;

    return 0;
}
```

```
'Hello, world!!!' size: 15
```
