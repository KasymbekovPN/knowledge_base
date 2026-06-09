---
tags:
  - programming-language
  - cpp
  - syntax
  - container
  - array
---
[[_cpp containers - array|<=]]

 Метод _swap_() меняет содержимое двух массивов.

```cpp
#include <iostream>
#include <array>

template<typename T, int N>
void print_array(const std::string& label, const std::array<T,N>& array) {
    std::cout << label << " : ";
    for (auto &&item: array) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}

int main() {
    std::array<int, 3> numbers0 {1, 2, 3};
    std::array<int, 3> numbers1 {4, 5, 6};

    print_array<int, 3>("numbers0", numbers0);
    print_array<int, 3>("numbers1", numbers1);

    numbers0.swap(numbers1);
    print_array<int, 3>("numbers0", numbers0);
    print_array<int, 3>("numbers1", numbers1);

    return 0;
}
```

```
numbers0 : 1 2 3 
numbers1 : 4 5 6
numbers0 : 4 5 6
numbers1 : 1 2 3
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
