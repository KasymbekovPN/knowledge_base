---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/std_string/find/_|<=]]

В C++ метод `substr()` класса `std::string` используется для получения **подстроки** из строки.

Если запросить подстроку с `len`, превышающим количество доступных символов после `pos`, `substr()` просто вернёт **всё, что осталось** , без ошибки.

```cpp
string substr(size_t pos = 0, size_t len = npos) const;
```

```cpp
#include <iostream>
#include <string>

void _test_sub_str(const std::string&, const size_t, const size_t);

int main() {
    const std::string str {"Hello"};
    for (size_t i {}; i < 5; i++) {
        _test_sub_str(str, 3, i);
    }

    return 0;
}

void _test_sub_str(const std::string& str,
				   const size_t pos,
				   const size_t len) {
    std::cout
        << "[" << pos << ", " << len << "] "
        << str.substr(pos, len) << std::endl;
}
```

```
[3, 0] 
[3, 1] l
[3, 2] lo
[3, 3] lo
[3, 4] lo
```
