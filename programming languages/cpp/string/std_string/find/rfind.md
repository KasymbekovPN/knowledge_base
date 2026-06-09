---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/std_string/find/_|<=]]

В C++ метод `rfind` у класса `std::string` используется для **поиска подстроки или символа** внутри строки с конца.

```cpp
size_t rfind(const string& str, size_t pos = 0) const noexcept;
size_t rfind(const char* s, size_t pos = 0) const;
size_t rfind(char c, size_t pos = 0) const;
```

Метод `std::string::rfind()` возвращает:
- **индекс последнего вхождения подстроки/символа** , если найдено,
- **`std::string::npos`** , если ничего не найдено.

```cpp
#include <iostream>
#include <string>

void _test_rfind(const std::string&, const std::string);

int main() {
    const std::string str {"hello"};
    _test_rfind(str, "l");
    _test_rfind(str, "x");

    return 0;
}

void _test_rfind(const std::string& str, const std::string sub) {
    auto idx = str.rfind(sub);
    if (idx != std::string::npos) {
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
