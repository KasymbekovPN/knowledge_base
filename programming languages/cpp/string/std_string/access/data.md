---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/std_string/access/_|<=]]

В C++ метод `data()` у класса `std::string` возвращает **указатель на внутренние данные строки** , то есть на массив символов `char`.

Важно: начиная с **C++17** , `data()` гарантирует, что возвращаемый указатель указывает на **null-terminated строку** .  
До C++17 — `data()` **мог быть не null-terminated** , если вы модифицировали строку так, чтобы она содержала внутри `'\0'`.

```cpp
#include <iostream>
#include <string>

void _test(const std::string&);

int main() {
    std::string str {"Hello World"};
    std::string nt_str = {"abc\0def"};

    _test(str);
    _test(nt_str);

    return 0;
}

void _test(const std::string& str) {
    std::cout << "#####" << std::endl;
    std::cout << "str <= " << str << std::endl;
    std::cout << "size <= " << str.size() << std::endl;

    const char* ptr = str.data();
    for (size_t i {}; i < str.size(); i++) {
        if (ptr[i] == '\0') {
            std::cout << "\\0";
        } else {
            std::cout << ptr[i];
        }
    }
    std::cout << std::endl;
}
```

```
#####
str <= Hello World
size <= 11
Hello World
#####
str <= abc
size <= 3
abc
```
