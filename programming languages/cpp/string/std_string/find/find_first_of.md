---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/std_string/find/_|<=]]

В C++ метод `find_first_of` у класса `std::string` используется для **поиска первого символа из набора** внутри строки с конца.

Метод `std::string::rfind()` возвращает:
- **индекс первого вхождения** , если найдено,
- **`std::string::npos`** , если ничего не найдено.

```cpp
#include <iostream>
#include <string>

void _test_find_first_of(const std::string&, const std::string);

int main() {
    const std::string str {"hello"};
    _test_find_first_of(str, "abc");
    _test_find_first_of(str, "el");
    _test_find_first_of(str, "l");

    return 0;
}

void _test_find_first_of(const std::string& str, const std::string sub) {
    auto idx = str.find_first_of(sub);
    if (idx != std::string::npos) {
        std::cout << "idx <= " << idx << std::endl;
    } else {
        std::cout << "NPOS" << std::endl;
    }
}
```

```
NPOS
idx <= 1
idx <= 2
```
