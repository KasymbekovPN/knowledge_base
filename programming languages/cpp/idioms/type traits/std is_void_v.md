---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

```cpp
#include <iostream>
#include <type_traits>

template<typename T>
void call_func(T (*func)()) {
    if constexpr (std::is_void_v<T>) {
        func();
        std::cout << "Ret. void" << std::endl;
    } else {
        std::cout
            << "Result: "
            << func()
            << std::endl;
    }
}

void ret_nothing() {
    std::cout
        << "Returns nothing"
        << std::endl;
}

int get_random() {
    return 42;
}

int main() {
    call_func(ret_nothing);
    call_func(get_random);

    return 0;
}
```

```
Returns nothing
Ret. void
Result: 42
```

