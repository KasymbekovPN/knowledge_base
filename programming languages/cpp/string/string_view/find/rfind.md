---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/string_view/find/_|<=]]

Метод `rfind()` ищет подстроку начиная с конца.

```cpp
#include <iostream>
#include <string>
#include <string_view>

void _test_rfind(const std::string_view&, const std::string);

int main() {
    const std::string_view sv {"hello"};
    _test_rfind(sv, "l");
    _test_rfind(sv, "x");

    return 0;
}

void _test_rfind(const std::string_view& sv, const std::string sub) {
    auto idx = sv.rfind(sub);
    if (idx != std::string_view::npos) {
        std::cout << "idx <= " << idx << std::endl;
    } else {
        std::cout << "NPOS" << std::endl;
    }
}
```

```
idx <= 3
NPOS
```
