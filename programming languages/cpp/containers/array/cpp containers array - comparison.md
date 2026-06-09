---
tags:
  - programming-language
  - cpp
  - syntax
  - container
  - array
---
[[_cpp containers - array|<=]]

Массивы можно сравнивать с помощью операторов `==`, `!=`, `<`, `>`, `<=`, `>=`. Сравнение выполняется лексикографически.

```cpp
#include <iostream>
#include <array>

int main() {
    std::array<int, 3> numbers0 {1, 2, 3};
    std::array<int, 3> numbers1 {1, 2, 4};
  
    if (numbers0 == numbers1) {
        std::cout << "numbers0 == numbers1" << std::endl;
    } else {
        std::cout << "numbers0 != numbers1" << std::endl;
    }

    if (numbers0 < numbers1) {
        std::cout << "numbers0 < numbers1" << std::endl;
    } else {
        std::cout << "numbers0 >= numbers1" << std::endl;
    }

    return 0;
}
```

```
numbers0 != numbers1
numbers0 < numbers1
```


---
---

### 6. **Обмен содержимого массивов**

#### a) **Метод `swap()`**
Меняет содержимое двух массивов.

```cpp
#include <iostream>
#include <array>

int main() {
    std::array<int, 3> arr1 = {1, 2, 3};
    std::array<int, 3> arr2 = {4, 5, 6};

    arr1.swap(arr2);

    std::cout << "arr1: ";
    for (int num : arr1) {
        std::cout << num << " ";
    }

    std::cout << "\narr2: ";
    for (int num : arr2) {
        std::cout << num << " ";
    }

    return 0;
}
```

**Вывод:**
```
arr1: 4 5 6 
arr2: 1 2 3
```

---

### 7. **Доступ к данным как к встроенному массиву**

#### a) **Метод `data()`**
Возвращает указатель на внутренний массив данных.

```cpp
#include <iostream>
#include <array>

int main() {
    std::array<int, 5> arr = {1, 2, 3, 4, 5};

    int* ptr = arr.data();

    for (size_t i = 0; i < arr.size(); ++i) {
        std::cout << ptr[i] << " ";
    }
    return 0;
}
```

**Вывод:**
```
1 2 3 4 5
```

---

### Итог
- `std::array` предоставляет удобные методы для доступа к элементам (`[]`, `at()`, `front()`, `back()`).
- Размер массива можно получить с помощью `size()`.
- Массив можно заполнить с помощью `fill()`.
- Массивы можно сравнивать и обменивать содержимым с помощью `swap()`.
- `std::array` безопаснее встроенных массивов, так как предоставляет методы для работы с размером и проверки границ.
