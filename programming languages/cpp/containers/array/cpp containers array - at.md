---
tags:
  - programming-language
  - cpp
  - syntax
  - container
  - array
---
[[_cpp containers - array|<=]]

Доступ к элементам по индексу с проверкой границ. Если индекс выходит за пределы массива, выбрасывается исключение `std::out_of_range`.

```cpp
#include <iostream>
#include <array>

int main() {
    const unsigned SIZE {2};
    std::array<int, SIZE> array {1, 2};

    for (unsigned i {}; i < SIZE; i++) {
        std::cout << "array[" << i << "] <= " << array.at(i) << std::endl;
    }

    try {
        std::cout << array.at(SIZE) << std::endl;
    }
    catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
    }

    return 0;
}
```

```
array[0] <= 1
array[1] <= 2
invalid array<T, N> subscript
```
