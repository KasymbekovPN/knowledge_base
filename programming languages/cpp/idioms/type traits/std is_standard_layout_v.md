---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]
### Что значит "standard layout"?

Тип имеет **стандартное расположение**, если его структура памяти совместима с C. Это позволяет:

- Передавать объекты в C-код,
- Использовать `memcpy` безопасно (в комбинации с `trivially_copyable`),
- Гарантировать предсказуемый порядок полей.

✅ Такие типы можно использовать в `extern "C"` и бинарных интерфейсах.

---

### Условия для `std::is_standard_layout_v<T> == true`

Тип `T` является standard-layout, если:
1. Все нестатические данные имеют одинаковую квалификацию доступа (`public`, `private`, `protected`)
2. Нет виртуальных функций
3. Нет виртуального наследования
4. Все непустые базовые классы также являются standard-layout
5. Объекты, ссылки и функции не могут быть полями

## ⚠️ Важные моменты

| Выражение                                       | Результат |
| ----------------------------------------------- | --------- |
| `std::is_standard_layout_v<int>`                | ✅ `true`  |
| `std::is_standard_layout_v<std::array<int, 3>>` | ✅ `true`  |
| `std::is_standard_layout_v<std::vector<int>>`   | ❌ `false` |
| `std::is_standard_layout_v<Point>`              | ✅ `true`  |
| `std::is_standard_layout_v<NonStandard>`        | ❌ `false` |

> 💡 Standard-layout + TriviallyCopyable = POD (до C++20)


```cpp
#include <iostream>
#include <type_traits>

struct Point {
    float x, y;
};

struct Base {
    float a;
};

struct Derived: Base {
    float b;
};

class NonStandard {
private:
    int x;
public:
    int y;
};

union U {
    int i;
    float f;
};

template<typename T>
void test_print(const std::string&&);

int main() {
    test_print<Point>("Point");
    test_print<Base>("Base");
    test_print<Derived>("Derived");
    test_print<NonStandard>("NonStandard");
    test_print<U>("U");

    return 0;
}

template<typename T>
void test_print(const std::string&& _lbl) {
    std::cout << _lbl;
    if constexpr (std::is_standard_layout_v<T>) {
        std::cout << " - std";
    } else {
        std::cout << " - no std";
    }
    std::cout << std::endl;
}
```

```
Point - std
Base - std
Derived - no std
NonStandard - no std
U - std
```

