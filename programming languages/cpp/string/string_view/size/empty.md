---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/string_view/size/_|<=]]

Метод `empty()` проверяет, пустая ли строка

```cpp
#include <iostream>
#include <string>
#include <string_view>

void _test_empty(const std::string_view&);

int main() {
    const std::string_view sv {"Example from literal"};
    const std::string_view empty_sv;

    _test_empty(sv);
    _test_empty(empty_sv);

    return 0;
}

void _test_empty(const std::string_view& sv) {
    std::cout
        << "Is it empty? "
        << std::boolalpha
        << sv.empty()
        << std::noboolalpha
        << std::endl;
}
```

```
Is it empty? false
Is it empty? true
```
