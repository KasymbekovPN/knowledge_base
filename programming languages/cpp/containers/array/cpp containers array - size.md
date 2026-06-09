---
tags:
  - programming-language
  - cpp
  - syntax
  - container
  - array
---
[[_cpp containers - array|<=]]

_size_ возвращает количество элементов в массиве.

```cpp
#include <iostream>
#include <array>


int main() {
    std::array<int, 5> numbers {1, 2, 3, 4, 5};
    std::cout << "size: " << numbers.size() << std::endl;

    return 0;
}
```

```
size: 5
```


---

#### b) **Метод `empty()`**
Проверяет, пуст ли массив. Для `std::array` всегда возвращает `false`, если размер массива больше 0.

```cpp
#include <iostream>
#include <array>

int main() {
    std::array<int, 5> arr = {1, 2, 3, 4, 5};

    std::cout << "Is empty: " << (arr.empty() ? "Yes" : "No") << "\n"; // No
    return 0;
}
```

**Вывод:**
```
Is empty: No
```

---

### 3. **Итерация по элементам**

#### a) **Использование range-based for**
```cpp
#include <iostream>
#include <array>

int main() {
    std::array<int, 5> arr = {1, 2, 3, 4, 5};

    for (int num : arr) {
        std::cout << num << " ";
    }
    return 0;
}
```

**Вывод:**
```
1 2 3 4 5
```

---

#### b) **Использование итераторов**
```cpp
#include <iostream>
#include <array>

int main() {
    std::array<int, 5> arr = {1, 2, 3, 4, 5};

    for (auto it = arr.begin(); it != arr.end(); ++it) {
        std::cout << *it << " ";
    }
    return 0;
}
```

**Вывод:**
```
1 2 3 4 5
```

---

### 4. **Заполнение массива**

#### a) **Метод `fill()`**
Заполняет все элементы массива указанным значением.

```cpp
#include <iostream>
#include <array>

int main() {
    std::array<int, 5> arr;
    arr.fill(42); // Заполняем массив значением 42

    for (int num : arr) {
        std::cout << num << " ";
    }
    return 0;
}
```

**Вывод:**
```
42 42 42 42 42
```

---

### 5. **Сравнение массивов**
Массивы можно сравнивать с помощью операторов `==`, `!=`, `<`, `>`, `<=`, `>=`. Сравнение выполняется лексикографически.

```cpp
#include <iostream>
#include <array>

int main() {
    std::array<int, 3> arr1 = {1, 2, 3};
    std::array<int, 3> arr2 = {1, 2, 4};

    if (arr1 == arr2) {
        std::cout << "arr1 == arr2\n";
    } else {
        std::cout << "arr1 != arr2\n";
    }

    if (arr1 < arr2) {
        std::cout << "arr1 < arr2\n";
    } else {
        std::cout << "arr1 >= arr2\n";
    }

    return 0;
}
```

**Вывод:**
```
arr1 != arr2
arr1 < arr2
```

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
