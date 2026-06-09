---
tags:
  - programming-language
  - cpp
  - syntax
  - container
  - deque
---
[[_cpp containers deque - access|<=]]

Доступ по индексу возможен через оператор _subscript_ (`[]`) . Попытка доступа к несуществующему элементу приводит к __неопределенному поведению__.

```cpp
#include <iostream>
#include <string>
#include <deque>

struct Result {
    bool success;
    int value;

    Result(bool success = false, int value = 0):
        success{success},
        value {value} {}
    std::string to_string() const {
        return "{success: " + std::to_string(success) +
                ", value: " + std::to_string(value) + "}";
    };
};

Result _subscript(const std::deque<int>&, unsigned);

int main(int argc, char const *argv[]) {
    std::deque<int> deq {1, 2, 3};
    for (unsigned i{}; i < 5; i++) {
        std::cout << _subscript(deq, i).to_string() << std::endl;
    }

    return 0;
}

Result _subscript(const std::deque<int>& deq, unsigned index) {
    return index < deq.size()
        ? Result{true, deq[index]}
        : Result{};
}
```

```
{success: 1, value: 1}
{success: 1, value: 2}
{success: 1, value: 3}
{success: 0, value: 0}
{success: 0, value: 0}
```
