---
tags:
  - programming-language
  - cpp
  - containers
---
[[_cpp containers span - access|<=]]

__Выполняется без проверки границ__

```cpp
#include <iostream>
#include <vector>
#include <span>

int main(int argc, char const *argv[]) {
    std::vector<int> vec {1, 2, 3};
    std::span<int> s0 {vec};

    for (size_t i {}; i < s0.size(); i++) {
        std::cout << s0[i] << std::endl;
    }

    return 0;
}
```

```
1
2
3
```

---

## Основные методы

### Доступ к элементам
```cpp
std::span<int> s = /* ... */;

// 2. at() (C++26)
try {
    int val = s.at(100);  // throws std::out_of_range
} catch (...) {}

// 3. front()/back()
int f = s.front();
int b = s.back();

// 4. data() - получение указателя
int* ptr = s.data();
```

### Итераторы
```cpp
// 1. begin()/end()
for (auto it = s.begin(); it != s.end(); ++it) { /* ... */ }

// 2. rbegin()/rend() (C++23)
for (auto it = s.rbegin(); it != s.rend(); ++it) { /* ... */ }

// 3. Range-based for
for (int x : s) { /* ... */ }
```

### Размер и емкость
```cpp
// 1. size()/size_bytes()
size_t count = s.size();
size_t bytes = s.size_bytes();

// 2. empty()
bool is_empty = s.empty();
```

### Под-диапазоны
```cpp
// 1. first() - первые N элементов
auto first3 = s.first(3);

// 2. last() - последние N элементов
auto last2 = s.last(2);

// 3. subspan() - произвольный диапазон
auto middle = s.subspan(1, 3);  // начиная с 1, длина 3
```



## Пример использования

```cpp
void process_data(std::span<const float> data) {
    for (float x : data) {
        // Обработка данных
    }
}

int main() {
    float arr[100];
    std::vector<float> vec(50);
    std::array<float, 10> std_arr;
    
    process_data(arr);      // C-массив
    process_data(vec);      // вектор
    process_data(std_arr);  // std::array
    
    // Часть данных
    process_data(std::span(vec).subspan(10, 20));
}
```

## Расширенные возможности (C++23)

1. **Конструирование из временных объектов**:
```cpp
std::span s = std::vector{1, 2, 3};  // Опасность! Временный объект
```

2. **Методы для работы с байтами**:
```cpp
std::span<std::byte> bytes = std::as_writable_bytes(s);
```

3. **Статические экстенты**:
```cpp
std::span<int, 3> fixed_size(arr);  // Размер известен в compile-time
```

`std::span` - это мощный инструмент для работы с непрерывными данными, который сочетает в себе безопасность и производительность.