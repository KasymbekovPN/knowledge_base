---
tags:
  - programming-language
  - cpp
  - containers
  - vector
---
[[_cpp containers|<=]]

Вектор представляет контейнер, который содержит коллекцию объектов одного типа. 

Для работы с векторами необходимо включить заголовок
```cpp
#include <vector>
```

```cpp
#include <iostream>
#include <vector>
#include <stdexcept>

void test_vector_by_idx(std::vector<int>*, size_t) noexcept;

int main(int argc, char const *argv[]) {
    std::vector<int> numbers {0, 1, 2, 3, 4};
    std::cout << "first <= " << numbers.front() << std::endl;
    std::cout << "second <= " << numbers[1] << std::endl;
    std::cout << "third <= " << numbers.at(2) << std::endl;
    std::cout << "last <= " << numbers.back() << std::endl;

    numbers[3] = -42;
    for (auto item: numbers) {
        std::cout << item << " ";
    }
    std::cout << std::endl;

    size_t last_size = std::size(numbers);
    ::test_vector_by_idx(&numbers, last_size);
    
    numbers.push_back(42);
    ::test_vector_by_idx(&numbers, last_size);
  
    return 0;
}

void test_vector_by_idx(std::vector<int>* pnumbers, size_t index) noexcept {
    try {
        int result = (*pnumbers).at(index);
        std::cout << "[test] " << result << std::endl;
    } catch(const std::out_of_range& e) {
        std::cerr << e.what() << '\n';
    }
}
```

```
first <= 0
second <= 1
third <= 2
last <= 4
0 1 2 -42 4
invalid vector subscript
[test] 42
```
- `[index]` - получение элемента по индексу (также как и в массивах), индексация начинается с нуля. Не добавляет элементов.
- `at(index)` - функция возвращает элемент по индексу. При попытке обращения по недопустимому индексу будет генерировать исключение _out_of_range_
- `front()` - возвращает первый элемент
- `back()` - возвращает последний элемент
- `push_back(value)` - добавляет в конец новый элемент

- [[cpp containers vectors - addition into vector|Addition elemenst into vector]]
- [[cpp containers vectors - insertion by position|Insertion by position]]
- [[cpp containers vectors - removing|Removing]]
- [[cpp containers vectors - resize|Resize]]
- [[cpp containers vectors - comparison|Comparison]]
- [[cpp containers vectors - swap|swap]]

---
[Вектор](https://metanit.com/cpp/tutorial/7.2.php)