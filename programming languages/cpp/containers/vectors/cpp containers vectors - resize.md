---
tags:
  - programming-language
  - cpp
  - containers
  - vector
---
[[_cpp containers vectors|<=]]

Метод _resize_ в __C++__ используется для изменения размера вектора (`std::vector`). Он может как увеличить, так и уменьшить размер вектора. Если новый размер больше текущего, новые элементы будут инициализированы значением по умолчанию или указанным значением. Если новый размер меньше текущего, лишние элементы будут удалены.

#### Увеличение размера вектора

```cpp
#include <iostream>
#include <vector>

void print_vector(const std::vector<int>&);

int main() {
    std::vector<int> numbers {1, 2, 3, 4, 5};
    print_vector(numbers);

    numbers.resize(7);
    print_vector(numbers);

    return 0;
}

void print_vector(const std::vector<int>& numbers) {
    std::cout << "size: " << numbers.size()
        << ", capacity: " << numbers.capacity()
        << " :: ";
    for (const auto& number: numbers) {
        std::cout << number << " ";
    }
    std::cout << std::endl;
}
```

```
size: 5, capacity: 5 :: 1 2 3 4 5 
size: 7, capacity: 7 :: 1 2 3 4 5 0 0
```

#### Увеличение размера с указанием значения для новых элементов

```cpp
#include <iostream>
#include <vector>

void print_vector(const std::vector<int>&);

int main() {
    std::vector<int> numbers {1, 2, 3, 4, 5};
    print_vector(numbers);

    numbers.resize(7, 123);
    print_vector(numbers);

    return 0;
}

void print_vector(const std::vector<int>& numbers) {
    std::cout << "size: " << numbers.size()
        << ", capacity: " << numbers.capacity()
        << " :: ";
    for (const auto& number: numbers) {
        std::cout << number << " ";
    }
    std::cout << std::endl;
}
```

```
size: 5, capacity: 5 :: 1 2 3 4 5 
size: 7, capacity: 7 :: 1 2 3 4 5 123 123
```

#### Уменьшение размера вектора

Если размер вектора уменьшается, элементы, выходящие за новый размер, теряются.

```cpp
#include <iostream>
#include <vector>

void print_vector(const std::vector<int>&);

int main() {
    std::vector<int> numbers {1, 2, 3, 4, 5};
    print_vector(numbers);

    numbers.resize(3);
    print_vector(numbers);

    numbers.shrink_to_fit();
    print_vector(numbers);

    return 0;
}

void print_vector(const std::vector<int>& numbers) {
    std::cout << "size: " << numbers.size()
        << ", capacity: " << numbers.capacity()
        << " :: ";
    for (const auto& number: numbers) {
        std::cout << number << " ";
    }
    std::cout << std::endl;
}
```

```
size: 5, capacity: 5 :: 1 2 3 4 5 
size: 3, capacity: 5 :: 1 2 3
size: 3, capacity: 3 :: 1 2 3
```

---
[Операции с векторами](https://metanit.com/cpp/tutorial/7.4.php)