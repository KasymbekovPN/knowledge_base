---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::is_pod_v<T>` из заголовка `<type_traits>` проверяет, является ли тип **POD** (Plain Old Data).
### Что такое POD?

**POD (Plain Old Data)** — это тип, который одновременно:

1. ✅ **Trivial**  
   → Имеет тривиальные конструкторы, деструктор, операторы копирования.

2. ✅ **Standard Layout**  
   → Имеет предсказуемое расположение полей (совместимо с C).

> 💡 Такие типы можно:
> - Копировать как блок байтов (`memcpy`),
> - Инициализировать через `{}`,
> - Передавать в C-код,
> - Размещать в статической памяти без инициализации.

## ⚠️ Важно: `std::is_pod_v` устарел в C++20

Начиная с **C++20**, `std::is_pod`, `std::is_pod_v` объявлены **устаревшими** (deprecated).  
Вместо них рекомендуется использовать комбинацию:

```cpp
std::is_trivial_v<T> && std::is_standard_layout_v<T>
```

> Но в C++11–C++17 он широко используется.

```cpp
#include <iostream>
#include <type_traits>

struct Point {
    float x, y;
};

struct Base {
    int a;
};

struct Derived: Base {
    int b;
};

class NonPod {
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
    test_print<NonPod>("NonPod");
    test_print<U>("U");

    return 0;
}

template<typename T>
void test_print(const std::string&& _lbl) {
    std::cout << _lbl;
    if constexpr (std::is_pod_v<T>) {
        std::cout << " - pod";
    } else {
        std::cout << " - no pod";
    }
    std::cout << std::endl;
}
```

```
Point - pod
Base - pod
Derived - no pod
NonPod - no pod
U - pod
```

