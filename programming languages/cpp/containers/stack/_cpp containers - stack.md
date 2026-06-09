---
tags:
  - programming-language
  - cpp
  - containers
  - stack
---
[[_cpp containers|<=]]

В C++ `std::stack` — это адаптер контейнера, который предоставляет интерфейс стека (LIFO — Last In, First Out). Он использует один из стандартных контейнеров (по умолчанию `std::deque`) для хранения элементов. Рассмотрим основные методы `std::stack`.

```cpp
#include <iostream>
#include <stack>

void _print_stack(std::stack<int>&);

int main(int argc, char const *argv[]) {
    std::stack<int> st;
    st.push(1);
    st.push(2);
    st.push(3);
    _print_stack(st);

    while (!st.empty()) {
        std::cout << st.top() << " ";
        st.pop();
    }
    std::cout << std::endl;
    _print_stack(st);

    return 0;
}

void _print_stack(std::stack<int>& st) {
    std::cout
        << "Size: " << st.size()
        << ", is empty" << std::boolalpha << ": " << st.empty()
        << std::noboolalpha
        << std::endl;
}
```

```
Size: 3, is empty: false
3 2 1
Size: 0, is empty: true
```
