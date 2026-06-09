---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/check/_|<=]]

Функция `std::equal` из заголовочного файла `<algorithm>` используется для **сравнения двух диапазонов** на равенство. Требуется, чтобы второй диапазон был достаточной длины, иначе - __неопределенное поведение__. 

```cpp
#include <algorithm>

// C++98/C++11
bool equal (InputIterator1 first1, InputIterator1 last1,
            InputIterator2 first2);

// C++14 и новее: с предикатом сравнения
bool equal (InputIterator1 first1, InputIterator1 last1,
            InputIterator2 first2,
            BinaryPredicate pred);
```

> ✅ Начиная с C++14, вы можете передать **предикат** — функцию или лямбду, которая будет использоваться для сравнения элементов.

### Важные моменты

| Особенность                                            | Объяснение                        |
| ------------------------------------------------------ | --------------------------------- |
| `std::equal` сравнивает элементы по одному             | Проверяет, что все пары равны     |
| Не проверяет длину контейнеров                         | Если второй диапазон короче — UB! |
| Можно использовать с любыми итераторами                | Включая массивы, строки, списки   |
| Начиная с C++14 — можно задать свой критерий сравнения | Лямбды, функции, функторы         |

### Как работает `std::equal`

- Проходит по первому диапазону `[first1, last1)`
- Для каждого элемента сравнивает его с соответствующим элементом во втором диапазоне (`first2 + i`)
- Если хотя бы одно сравнение не проходит — возвращает `false`
- Иначе — возвращает `true`

### Простой

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

void _test_equal(const std::vector<int>&, const std::vector<int>&);

int main() {
    std::vector<int> v0 = {1, 2, 3, 4};
    std::vector<int> v1 = {1, 2, 3, 4};
    std::vector<int> v2 = {1, 2, 3, 5};
    _test_equal(v0, v1);
    _test_equal(v0, v2);

    return 0;
}

void _test_equal(const std::vector<int>& v0, const std::vector<int>& v1) {
    if (std::equal(v0.begin(), v0.end(), v1.begin())) {
        std::cout << "Vectorts are equal" << std::endl;
    } else {
        std::cout << "Vectorts are not equal" << std::endl;
    }
}
```

```
Vectorts are equal
Vectorts are not equal
```

### С предикатом 

```cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

int main() {
    const std::vector<int> v0 {10, 20, 30};
    const std::vector<int> v1 {12, 18, 32};

    bool result = std::equal(
        v0.begin(),
        v0.end(),
        v1.begin(),
        [](int x, int y) {return std::abs(x - y) <= 2;});

    std::cout
        << "result <= "
        << std::boolalpha
        << result
        << std::noboolalpha
        << std::endl;

    return 0;
}
```

```
result <= true
```


---

### Вывод:
```
Strings are equal (case-insensitive)
```

---

## 📝 Вывод

| Форма                                                | Что делает                                     |
| ---------------------------------------------------- | ---------------------------------------------- |
| `std::equal(b1, e1, b2)`                             | Сравнивает два диапазона стандартным способом  |
| `std::equal(b1, e1, b2, pred)`                       | Сравнивает с помощью пользовательского условия |
| Требует, чтобы второй диапазон был достаточной длины | Иначе — **неопределённое поведение**           |
