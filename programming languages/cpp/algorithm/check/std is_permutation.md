---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/check/_|<=]]

Функция `std::is_permutation` из заголовочного файла `<algorithm>` используется для проверки, является ли **одна последовательность перестановкой другой**.

> 💡 То есть: все элементы первой последовательности должны присутствовать во второй, и их количество должно совпадать, но порядок может быть любым.

### Синтаксис

```cpp
#include <algorithm>

// C++11 и выше
bool is_permutation(ForwardIterator1 first1, ForwardIterator1 last1,
                    ForwardIterator2 first2);

// C++14 и выше — с предикатом сравнения
bool is_permutation(ForwardIterator1 first1, ForwardIterator1 last1,
                    ForwardIterator2 first2,
                    BinaryPredicate pred);
```

- `first1`, `last1`: итераторы, определяющие первый диапазон.
- `first2`: начало второго диапазона.
- `pred`: необязательный бинарный предикат (например, лямбда) для пользовательского сравнения.

### Важные моменты

| Особенность                                                  | Объяснение                                                 |
| ------------------------------------------------------------ | ---------------------------------------------------------- |
| Не требует, чтобы диапазоны были отсортированы               | Работает с любыми последовательностями                     |
| Проверяет полное соответствие по количеству и типу элементов | Если одинаковые элементы, но разная длина — вернёт `false` |
| Может быть медленным на больших данных                       | Это O(n²) алгоритм в худшем случае                         |
| Первый диапазон задаётся через `[first1, last1)`             | Второй диапазон должен быть как минимум такой же длины     |
| Начиная с C++14 можно использовать custom predicate          | Полезно для сложных объектов или неточного сравнения       |


### Простой пример

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

void _test_is_permutation(const std::vector<int>&, const std::vector<int>&);

int main() {
    const std::vector<int> v0 {1, 2, 3};
    const std::vector<int> v1 {2, 3, 1};
    const std::vector<int> v2 {0, 1, 2};

    _test_is_permutation(v0, v1);
    _test_is_permutation(v1, v2);

    return 0;
}

void _test_is_permutation(const std::vector<int>& v0,
						  const std::vector<int>& v1) {
    if (std::is_permutation(v0.begin(), v0.end(), v1.begin())) {
        std::cout << "Permutation" << std::endl;
    } else {
        std::cout << "NOT permutation" << std::endl;
    }
}
```

### С предикатом

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

void _test_is_permutation(const std::vector<int>&, const std::vector<int>&);
bool _abs_equal(int, int);

int main() {
    const std::vector<int> v0 {1, 2, 3};
    const std::vector<int> v1 {2, 3, 1};
    const std::vector<int> v2 {0, 1, 2};

    _test_is_permutation(v0, v1);
    _test_is_permutation(v1, v2);

    return 0;
}

void _test_is_permutation(const std::vector<int>& v0,
						  const std::vector<int>& v1) {
    bool result = std::is_permutation (
        v0.begin(),
        v0.end(),
        v1.begin(),
        _abs_equal
    );
    std::cout
        << "Permutation <= "
        << std::boolalpha
        << result
        << std::noboolalpha
        << std::endl;
}

bool _abs_equal(int a, int b) {
    return abs(a) == abs(b);
}
```

```
Permutation <= true
Permutation <= false
```
