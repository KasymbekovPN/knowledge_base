---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/string_view/access/_|<=]]

В C++ метод **`operator[]`** у `std::string_view` позволяет получить доступ к символу по индексу, **без проверки на выход за границы** . Это делает его **быстрым** , но потенциально **небезопасным**  (неопределенное поведение).

```cpp
#include <iostream>
#include <string>
#include <string_view>

int main() {
    const std::string_view sv {"ABC"};
    for (size_t i {}; i < sv.size(); i++) {
        std::cout << sv[i];
    }
    std::cout << std::endl;

    return 0;
}
```

```
ABC
```
