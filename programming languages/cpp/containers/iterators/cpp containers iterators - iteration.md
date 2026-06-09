---
tags:
  - programming-language
  - cpp
  - container
  - iterator
---
[[_cpp containers iterators|<=]]

При работе с контейнерами следует учитывать, что добавление или удаление элементов в контейнере может привести к тому, что все текущие итераторы для данного контейнера, а также ссылки и указатели на его элементы станут недопустимыми. Поэтому при добавлении или удалении элементов в контейнере в общем случае следует перестать использовать текущие итераторы для этого контейнера.

```cpp
#include <iostream>
#include <vector>

int main(int argc, char const *argv[]) {
    std::vector<int> numbers {1, 2, 3};

    auto it {numbers.begin()};
    while (it != numbers.end()) {
        std::cout << "[while] value <= " << *it << std::endl;
        ++it;
    }

    for (auto it {numbers.begin()}; it != numbers.end(); it++) {
        std::cout << "[for] value <= " << *it << std::endl;
    }

    return 0;
}
```

```
[while] value <= 1
[while] value <= 2
[while] value <= 3
[for] value <= 1
[for] value <= 2
[for] value <= 3
```

---
[Итераторы](https://metanit.com/cpp/tutorial/7.3.php)