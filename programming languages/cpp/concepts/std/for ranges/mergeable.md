---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for ranges/_|<=]]

`std::mergeable` — concept из C++20 (`<iterator>`), который используется алгоритмами слияния отсортированных последовательностей, например:

- `std::ranges::merge`
- `std::ranges::set_union`
- `std::ranges::set_intersection`
- `std::ranges::set_difference`

Он проверяет:
> можно ли читать элементы из двух входных диапазонов, сравнивать их и записывать результат в выходной диапазон.

# Определение

Упрощённо стандарт определяет его примерно так:

```cpp
template<
    class I1,
    class I2,
    class Out,
    class Comp = ranges::less,
    class Proj1 = std::identity,
    class Proj2 = std::identity>
concept mergeable =
    std::input_iterator<I1> &&
    std::input_iterator<I2> &&
    std::weakly_incrementable<Out> &&

    std::indirectly_copyable<I1, Out> &&
    std::indirectly_copyable<I2, Out> &&

    std::indirect_strict_weak_order<
        Comp,
        std::projected<I1, Proj1>,
        std::projected<I2, Proj2>
    >;
```

# Что требует mergeable

## 1. Два входных итератора

```cpp
I1
I2
```

из которых можно читать значения.

## 2. Выходной итератор

```cpp
Out
```

в который можно писать результат.

## 3. Возможность копирования

Должно работать:

```cpp
*out = *it1;
```

и

```cpp
*out = *it2;
```

## 4. Возможность сравнения

Должно работать:

```cpp
comp(*it1, *it2);
```

# Связь с другими concepts

`mergeable` включает несколько уже знакомых concepts:

```text
input_iterator
        +
indirectly_copyable
        +
indirect_strict_weak_order
        ↓
    mergeable
```

# Где используется

Основные алгоритмы:
- std::ranges::merge
- std::ranges::set_union
- std::ranges::set_intersection
- std::ranges::set_difference
- std::ranges::includes

Все они работают с двумя отсортированными диапазонами и используют сравнение элементов.

# Итог

`std::mergeable<I1, I2, Out>` гарантирует, что:
- `I1` и `I2` являются входными итераторами;
- элементы из обоих диапазонов можно читать;
- элементы можно копировать в `Out`;
- элементы можно сравнивать через компаратор;
- итераторы подходят для алгоритмов слияния отсортированных последовательностей (`merge`, `set_union`, `set_intersection` и др.).

```cpp
#include <iostream>
#include <iterator>
#include <vector>
#include <algorithm>

struct Value {
    int x{};
  
    Value(int _x): x{_x} {}

    bool operator==(const Value&) const = default;
    auto operator<=>(const Value&) const = default;
};

std::ostream& operator<<(std::ostream& _os, const Value& _value) {
    return _os << "{" << _value.x << "}";
}

template<typename T>
requires std::mergeable<
    typename std::vector<T>::iterator,
    typename std::vector<T>::iterator,
    std::back_insert_iterator<std::vector<T>>
>
void test(std::vector<T>& _in0,
          std::vector<T>& _in1,
          std::vector<T>& _out) {
    std::ranges::merge(
        _in0,
        _in1,
        std::back_inserter(_out)
    );
}

template<typename T>
void print(std::vector<T>& _vec) {
    for (auto& item: _vec) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}

int main() {
    std::vector<int> vin0 {1, 2, 3};
    std::vector<int> vin1 {3, 4, 5};
    std::vector<int> vout;
    test(vin0, vin1, vout);
    print(vout);

    std::vector<Value> vain0 {{0}, {1}, {2}};
    std::vector<Value> vain1 {{2}, {3}, {4}};
    std::vector<Value> vaout;;
    test(vain0, vain1, vaout);
    print(vaout);

    return 0;
}
```

```
1 2 3 3 4 5 
{0} {1} {2} {2} {3} {4}
```
