---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/std_string/find/_|<=]]

В C++ метод `find` у класса `std::string` используется для **поиска подстроки или символа** внутри строки.

```cpp
size_t find(const string& str, size_t pos = 0) const noexcept;
size_t find(const char* s, size_t pos = 0) const;
size_t find(char c, size_t pos = 0) const;
```

Метод `std::string::find()` возвращает:
- **индекс первого вхождения подстроки/символа** , если найдено,
- **`std::string::npos`** , если ничего не найдено.

```cpp
#include <iostream>
#include <string>

void _test_find(const std::string&, const std::string);

int main() {
    const std::string str {"hello"};
    _test_find(str, "l");
    _test_find(str, "x");

    return 0;
}

void _test_find(const std::string& str, const std::string sub) {
    auto idx = str.find(sub);
    if (idx != std::string::npos) {
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
