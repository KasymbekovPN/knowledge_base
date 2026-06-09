---
tags:
  - programming-language
  - cpp
  - containers
  - map
---
[[_cpp containers unordered_map - size|<=]]

Коэффициент загрузки - это отношение количества элементов в контейнере к количеству корзин (buckets)

Когда `load_factor() > max_load_factor()`, происходит **рехеширование**:
- Увеличивается количество корзин
- Все элементы перераспределяются по новым корзинам

Типичные значения:
- По умолчанию `max_load_factor()` обычно 1.0
- Оптимальное значение обычно между 0.7 и 0.8

```cpp
#include <iostream>
#include <unordered_map>

int main() {
    std::unordered_map<int, int> map {
        {1, 1},
        {2, 2},
        {3, 3}
    };
    std::cout << "LF: " << map.load_factor() << std::endl;

    // map.max_load_factor(0.5);
    map.emplace(4, 4);
    std::cout << "LF: " << map.load_factor() << std::endl;

    return 0;
}
```

```
LF: 0.375
LF: 0.5
```
