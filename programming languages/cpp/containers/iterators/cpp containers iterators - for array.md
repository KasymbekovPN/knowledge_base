---
tags:
  - programming-language
  - cpp
  - container
  - iterator
  - array
---
[[_cpp containers iterators|<=]]

Для массивов в __C++__ также имеется поддержка итераторов. Для этого в стандартной библиотеке __С++__ определены функции _std::begin()_ и _std::end()_.

Как и контейнеры, массив можно перебрать с помощью итераторов.

Но перебор массива вполне можно сделать и другими способами - через индексы, обычные указатели. Но итераторы на массивы могут быть полезны при манипуляции с контейнерами. Например, функция _insert()_, которая есть у ряда контейнеров, позволяет добавить в контейнер какую-то часть другого контейнера. Для выделения добавляемой части могут применяться итераторы. И таким образом, с помощью итераторов можно добавить в контейнер, например, в вектор какую-то часть контейнера.

```cpp
#include <iostream>
#include <vector>

int main(int argc, char const *argv[]) {
    int data[] {100, 101, 102, 103};
    for (auto it {std::begin(data)}; it != std::end(data); it++) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;

    std::vector<int> numbers {1, 2, 3};
    numbers.insert(numbers.end(), std::begin(data) + 1, std::end(data) - 1);
    for (auto it {numbers.begin()}; it != numbers.end(); it++) {
        std::cout << *it << " ";
    }

    return 0;
}
```

```
100 101 102 103 
1 2 3 101 102
```

---
[Итераторы](https://metanit.com/cpp/tutorial/7.3.php)