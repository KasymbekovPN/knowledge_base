---
tags:
  - programming-language
  - cpp
  - containers
  - vector
---
[[_cpp containers vectors|<=]]

Метод _swap_ в __C++__ используется для обмена содержимого двух объектов. Для _std::vector_ метод _swap_ позволяет быстро обменять содержимое двух векторов. Это очень эффективная операция, так как она не копирует элементы, а просто меняет внутренние указатели векторов. Не копирует элементы, поэтому операция выполняется за константное время O(1)

```cpp
#include <iostream>
#include <vector>

void print_vector(const std::string&, const std::vector<int>&);

int main() {
    std::vector<int> numbers0 {0, 1, 2, 3, 4};
    std::vector<int> numbers1 {5, 6, 7, 8, 9};

    std::cout << "original:" << std::endl;
    print_vector("numbers0", numbers0);
    print_vector("numbers1", numbers1);

    std::cout << "after swap:" << std::endl;
    numbers0.swap(numbers1);
    print_vector("numbers0", numbers0);
    print_vector("numbers1", numbers1);

    std::cout << "after swap with empty vector:" << std::endl;
    std::vector<int>().swap(numbers0);
    print_vector("numbers0", numbers0);

    return 0;
}

void print_vector(const std::string& label,
				  const std::vector<int>& numbers) {
    std::cout << "[" << label << "] size: " << numbers.size()
        << ", capacity: " << numbers.capacity()
        << " :: ";
    for (const auto& number: numbers) {
        std::cout << number << " ";
    }
    std::cout << std::endl;
}
```

```
original:
[numbers0] size: 5, capacity: 5 :: 0 1 2 3 4
[numbers1] size: 5, capacity: 5 :: 5 6 7 8 9
after swap:
[numbers0] size: 5, capacity: 5 :: 5 6 7 8 9
[numbers1] size: 5, capacity: 5 :: 0 1 2 3 4
after swap with empty vector:
[numbers0] size: 0, capacity: 0 ::
```

---
[Операции с векторами](https://metanit.com/cpp/tutorial/7.4.php)