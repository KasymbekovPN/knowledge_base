---
tags:
  - programming-language
  - cpp
  - syntax
  - container
  - forward_list
---
[[_cpp containers forward_list - removing|<=]]

Метод _erase_after()_ удаляет элемент после указанной позиции. Метод _erase_after_ может удалить диапазон элементов. Попытка вызова на не существующем элементе приводит к __неопределенному поведению__.

```cpp
#include <iostream>
#include <forward_list>

template<typename T>
void print_flist(const std::forward_list<T>&);

int main() {
    std::forward_list<int> numbers {1, 2, 3, 4, 5, 6, 7};
    print_flist(numbers);

    auto numberIt = numbers.begin();
    std::advance(numberIt, 2);
    numbers.erase_after(numberIt);
    print_flist(numbers);

    auto startIt = numbers.begin();
    auto finishIt = numbers.begin();
    std::advance(startIt, 2);
    std::advance(finishIt, 5);
    numbers.erase_after(startIt, finishIt);
    print_flist(numbers);

    return 0;
}

template<typename T>
void print_flist(const std::forward_list<T>& flist) {
    std::cout << "flist => ";
    for (const auto &item: flist) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}
```

```
flist => 1 2 3 4 5 6 7 
flist => 1 2 3 5 6 7
flist => 1 2 3 7
```
