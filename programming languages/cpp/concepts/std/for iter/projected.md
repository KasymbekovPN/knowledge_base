---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for iter/_|<=]]

`std::projected` — это не concept, а **метафункция (alias template)** из `<iterator>` в C++20.

Она используется в ranges-алгоритмах для описания типа, который получается после применения **проекции** к элементу диапазона.
# Что такое проекция (projection)

Представим структуру:

```cpp
struct Person {
    std::string name;
    int age;
};
```

и вектор:

```cpp
std::vector<Person> people;
```

Если вы хотите сортировать по возрасту, то вместо компаратора:

```cpp
[](const Person& a, const Person& b) {
    return a.age < b.age;
}
```

можно использовать проекцию:

```cpp
&Person::age
```

# Связь с concepts

`std::projected` часто встречается внутри требований таких concepts:
- `std::sortable`
- `std::indirect_unary_predicate`
- `std::indirect_binary_predicate`
- `std::indirectly_comparable`
- `std::mergeable`

Например:

```cpp
std::indirect_unary_predicate<
    Pred,
    std::projected<I, Proj>
>
```

означает:
> "Pred можно вызвать для элемента после применения проекции".

```cpp
#include <iostream>
#include <iterator>
#include <vector>
#include <algorithm>

struct Person {
    std::string name;
    int age;
};

int main() {
    std::vector<Person> people {
        {"Bob", 30},
        {"Alice", 20},
        {"John", 25}
    };

    auto&&proj = &Person::age;
    auto&& result = std::invoke(proj, *people.begin());
    std::cout << result << std::endl;

    std::ranges::sort(
        people,
        std::ranges::less(),
        &Person::age
    );
    for (const auto &p: people) {
        std::cout
            << "{name: '" << p.name
            << "', age: " << p.age << "}" << std::endl;
    }

    return 0;
}
```

```
30
{name: 'Alice', age: 20}
{name: 'John', age: 25}
{name: 'Bob', age: 30}
```
