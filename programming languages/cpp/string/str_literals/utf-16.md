---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/str_literals/_|<=]]

- Каждый символ имеет размер **2 байта** (в Windows) или **4 байта** (в Unix).
- Тип: `const char16_t*`.

```cpp
#include <windows.h>
#include <iostream>
#include <locale>
#include <codecvt>
#include <string>

int main() {
    SetConsoleOutputCP(CP_UTF8);

    const char16_t* str = u"你好世界!";
    std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> conv;
    std::string utf8 = conv.to_bytes(str);
    std::cout << utf8 << std::endl;

    return 0;
}
```

```
你好世界!
```
