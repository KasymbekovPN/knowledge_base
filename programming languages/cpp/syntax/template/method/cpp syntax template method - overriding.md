---
tags:
  - programming-language
  - cpp
  - syntax
  - template
  - method
---
[[_cpp syntax template|<=]]

С одной стороны, параметризация позволяет снизить возможность перегрузки функций, так как мы можем абстрагироваться от конкретных типов. С другой стороны, все равно могут быть ситуации, когда необходимы разные версии функции. И тут мы можем совместить перегрузку функций и их параметризацию.

```cpp
#include <iostream>

template<typename T> const T* max(const T*, const T*);
template<typename T> const T* max(const T[], unsigned);

int main(int argc, char const *argv[]) {
    const int A{42};
    const int B{45};
    std::cout << "first variant:\t" << *max(&A, &B) << std::endl;

    const double NUMBERS[] {1.2, 2.3, 3.4, 4.5};
    std::cout
	    << "second variant:\t"
	    << *max(NUMBERS, std::size(NUMBERS))
	    << std::endl;

    return 0;
}

template<typename T> const T* max(const T* a, const T* b) {
    return *a > *b ? a : b;
}

template<typename T> const T* max(const T data[], unsigned size) {
    const T* result {};
    for (size_t i{}; i < size; i++) {
        if (!result || data[i] > *result) {
            result = &data[i];
        }
    }

    return result;
}
```

```
first variant:  45
second variant: 4.5
```

---
[Шаблоны функций](https://metanit.com/cpp/tutorial/9.2.php)