---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for functional/_|<=]]

`std::strict_weak_order` — concept из `<concepts>`, который описывает **отношение строгого слабого порядка** (strict weak ordering).

Именно такой компаратор требуется большинству алгоритмов сортировки STL:
- `std::sort`
- `std::ranges::sort`
- `std::ranges::lower_bound`
- `std::ranges::upper_bound`
- `std::ranges::binary_search`
- и другим алгоритмам упорядоченных диапазонов.

# Определение

Формально:

```cpp
template<class R, class T, class U>
concept strict_weak_order =
    std::relation<R, T, U>;
```

На уровне синтаксиса компилятор проверяет только то же самое, что и [[relation|std::relation]].

Однако стандарт накладывает дополнительные **семантические требования**.

# Что означает строгий слабый порядок

Для компаратора `comp(a, b)` должны выполняться свойства:

## 1. Иррефлексивность

Объект не меньше самого себя:

```cpp
comp(x, x) == false
```

Пример:

```cpp
5 < 5
```

даёт

```cpp
false
```

## 2. Асимметричность

Если:

```cpp
comp(a, b) == true
```

то:

```cpp
comp(b, a) == false
```

Например:

```cpp
5 < 10
```

истина.

Тогда:

```cpp
10 < 5
```

обязательно ложь.

## 3. Транзитивность

Если:

```cpp
comp(a, b)
comp(b, c)
```

то:

```cpp
comp(a, c)
```

тоже должно быть true.

Например:

```cpp
1 < 2
2 < 3
```

следовательно:

```cpp
1 < 3
```

## 4. Эквивалентность

Если:

```cpp
!comp(a,b)
!comp(b,a)
```

то элементы считаются эквивалентными.

# Связь с другими concepts

```text
invocable
    ↓
regular_invocable
    ↓
predicate
    ↓
relation
    ↓
strict_weak_order
```

# Итог

`std::strict_weak_order<R, T, U>` означает:
- `R` является бинарным предикатом для `T` и `U`;
- корректны вызовы `(T,T)`, `(U,U)`, `(T,U)`, `(U,T)`;
- результат приводится к `bool`;
- семантически компаратор задаёт строгий слабый порядок:
    - `comp(x,x) == false`;
    - соблюдается асимметричность;
    - соблюдается транзитивность;
- именно такие компараторы должны использоваться для сортировки и поиска в упорядоченных диапазонах.
