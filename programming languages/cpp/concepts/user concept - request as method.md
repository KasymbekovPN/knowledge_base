---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/_|<=]]

```cpp
#include <iostream>
#include <concepts>
#include <vector>

template<typename T>
concept Container = requires(T c) {
    typename T::value_type;
    {c.begin()} -> std::input_iterator;
    {c.end()} -> std::same_as<decltype(c.begin())>;
    {c.size()} -> std::convertible_to<size_t>;
};

template<Container C>
void test(const C&);

int main() {
    test(std::vector<int>({1, 2, 3}));
    // test(42); // Error

    return 0;
}

template<Container C>
void test(const C& _container) {
    std::cout << "Size: " << _container.size() << std::endl;
}
```

```
Size: 3
```

```
.\as_method.cpp:18:5: error: no matching function for call to 'test'
   18 |     test(42); // Error
      |     ^~~~
.\as_method.cpp:14:6: note: candidate template ignored: constraints not satisfied [with C = int]
   14 | void test(const C&);
      |      ^
.\as_method.cpp:13:10: note: because 'int' does not satisfy 'Container'
   13 | template<Container C>
      |          ^
.\as_method.cpp:7:14: note: because 'typename T::value_type' would be invalid: type 'int' cannot be used prior to '::' because it has no members
    7 |     typename T::value_type;
      |              ^
```
