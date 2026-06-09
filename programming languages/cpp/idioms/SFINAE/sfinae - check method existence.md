---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/SFINAE/_|<=]]

```cpp
#include <iostream>
#include <type_traits>

template<typename T>

class has_serialize {
    template<typename U>
    static auto test(int) -> decltype(std::declval<U>().serialize(), std::true_type{});

    template<typename U>
    static std::false_type test(...);

public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

struct A {
    void serialize() const {
        std::cout << "A ser" << std::endl;
    }
};

struct B {};

int main() {
    std::cout
        << std::boolalpha
        << "A has serialize: "
        << has_serialize<A>::value
        << std::endl;
    std::cout
        << "B has serialize: "
        << has_serialize<B>::value
        << std::noboolalpha
        << std::endl;

    return 0;
}
```

```
A has serialize: true
B has serialize: false
```
