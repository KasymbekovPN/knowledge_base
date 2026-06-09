---
tags:
  - programming-language
  - cpp
  - containers
  - vector
---
[[_cpp containers vectors|<=]]

В __C++__ векторы (`std::vector`) можно сравнивать с помощью операторов сравнения (`==`, `!=`, `<`, `>`, `<=`, `>=`). Эти операторы сравнивают векторы **лексикографически**, то есть поэлементно, начиная с первого элемента.

```cpp
#include <iostream>
#include <vector>

int main() {
    std::vector<int> numbers1 = {1, 2, 3};
    std::vector<int> numbers2 = {1, 2, 3};
    std::vector<int> numbers3 = {1, 2, 4};
    std::vector<int> numbers4 = {1, 2};
    std::vector<int> numbers5 = {1, 2, 3, 4};

    std::cout
        << "numbers1 == numbers2 -> "
        << std::boolalpha
        << (numbers1 == numbers2)
        << std::noboolalpha
        << std::endl;

    std::cout
        << "numbers1 != numbers3 -> "
        << std::boolalpha
        << (numbers1 != numbers3)
        << std::noboolalpha
        << std::endl;

    std::cout
        << "numbers1 < numbers3 -> "
        << std::boolalpha
        << (numbers1 < numbers3)
        << std::noboolalpha
        << std::endl;

    std::cout
        << "numbers4 <= numbers1 -> "
        << std::boolalpha
        << (numbers4 <= numbers1)
        << std::noboolalpha
        << std::endl;

    std::cout
        << "numbers5 > numbers1 -> "
        << std::boolalpha
        << (numbers5 > numbers1)
        << std::noboolalpha
        << std::endl;

    return 0;

}
```

```
numbers1 == numbers2 -> true
numbers1 != numbers3 -> true
numbers1 < numbers3 -> true
numbers4 <= numbers1 -> true
numbers5 > numbers1 -> true
```

##### Лексикографическое сравнение
- Векторы сравниваются поэлементно, начиная с первого элемента.
- Если элементы на текущей позиции равны, сравниваются следующие элементы.
- Если один вектор является префиксом другого, более короткий вектор считается меньшим.

##### Примеры
- `{1, 2, 3} == {1, 2, 3}` → `true`
- `{1, 2, 3} != {1, 2, 4}` → `true`
- `{1, 2, 3} < {1, 2, 4}` → `true` (поскольку `3 < 4`)
- `{1, 2} <= {1, 2, 3}` → `true` (поскольку `{1, 2}` — префикс `{1, 2, 3}`)
- `{1, 2, 3, 4} > {1, 2, 3}` → `true` (поскольку `{1, 2, 3, 4}` длиннее)

---
[Операции с векторами](https://metanit.com/cpp/tutorial/7.4.php)