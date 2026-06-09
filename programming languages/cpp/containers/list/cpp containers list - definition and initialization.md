---
tags:
  - programming-language
  - cpp
  - syntax
  - container
  - list
---
[[_cpp containers list|<=]]

```cpp
#include <iostream>
#include <list>

void print_list(const std::list<int>&, const std::string&);

int main() {
    std::list<int> list0;
    std::list<int> list1 {};
    std::list<int> list2 {1, 2, 3};

    print_list(list0, "list0");
    print_list(list1, "list1");
    print_list(list2, "list2");

    return 0;
}

void print_list(const std::list<int>& list, const std::string &label) {
    std::cout << label << "[" << list.size() << "]: ";
    for (auto &&item: list) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}
```

```
list0[0]: 
list1[0]:
list2[3]: 1 2 3
```
