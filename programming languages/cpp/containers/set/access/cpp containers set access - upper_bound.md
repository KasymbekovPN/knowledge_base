---
tags:
  - programming-language
  - cpp
  - container
  - set
---
[[_cpp containers set - access|<=]]

Метод `upper_bound(const T& value)` возвращает итератор на первый элемент, который больше `value`.

```cpp
#include <iostream>
#include <set>

void _test_upper_bound_result(const std::set<int>::iterator&,
                              const std::set<int>&);
int main() {
    std::set<int> s {1, 2, 3, 4, 5};
    _test_upper_bound_result(s.upper_bound(2), s);
    _test_upper_bound_result(s.upper_bound(4), s);
    _test_upper_bound_result(s.upper_bound(5), s);
    _test_upper_bound_result(s.upper_bound(42), s);

    return 0;
}

void _test_upper_bound_result(const std::set<int>::iterator& it,
                              const std::set<int>& set)
{
    if (set.end() == it) {
        std::cout << "END";
    } else {
        std::cout << *it;
    }
    std::cout << std::endl;
}
```

```
3
5
END
END
```