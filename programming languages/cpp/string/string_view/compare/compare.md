---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/string_view/find/_|<=]]

Метод `compare()` выполнение сравнения строк.

```cpp
#include <iostream>
#include <string>
#include <string_view>

int main() {
    const std::string_view a {"a"};
    const std::string_view b {"b"};
    const std::string_view c {"c"};

    std::cout << std::boolalpha;
    std::cout
        << a << " < "  << b << " => "
        << (a < b) << std::endl;
    std::cout
        << a << " > "  << b << " => "
        << (a > b) << std::endl;
    std::cout
        << a << " == " << b << " => "
        << (a == b) << std::endl;
    std::cout << std::noboolalpha;

    std::cout
        << b << ".compare(" << a << ")"
        << " => " << b.compare(a)
        << std::endl;
    std::cout
        << b << ".compare(" << b << ")"
        << " => " << b.compare(b)
        << std::endl;
    std::cout
        << b << ".compare(" << c << ")"
        << " => " << b.compare(c)
        << std::endl;

    return 0;
}
```

```
a < b => true
a > b => false
a == b => false
b.compare(a) => 1
b.compare(b) => 0
b.compare(c) => -1
```
