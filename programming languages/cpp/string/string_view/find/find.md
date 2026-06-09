---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/string_view/find/_|<=]]

Метод `find()` ищет подстроку.

```cpp
#include <iostream>
#include <string>
#include <string_view>

void _test_find(const std::string_view&, const std::string);

int main() {
    const std::string_view sv {"hello"};
    _test_find(sv, "l");
    _test_find(sv, "x");

    return 0;
}

void _test_find(const std::string_view& sv, const std::string sub) {
    auto idx = sv.find(sub);
    if (idx != std::string_view::npos) {
        std::cout << "idx <= " << idx << std::endl;
    } else {
        std::cout << "NPOS" << std::endl;
    }
}
```

```
idx <= 2
NPOS
```
