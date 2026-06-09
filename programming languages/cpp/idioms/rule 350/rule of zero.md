---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/rule 350/_|<=]]

**Лучший подход**: **не писать ни одного** из этих методов вручную!

Вместо управления памятью напрямую — используйте **умные указатели** и **стандартные контейнеры**.

### ✅ Пример с Rule of Zero

```cpp
#include <string>
#include <memory>

class Wrapper {

private:
    std::string data;

public:
    Wrapper(const char* _str): data{_str} {}
    ~Wrapper() {}
};
```

✅ `std::string` уже следует Rule of Five — не нужно это повторять.
