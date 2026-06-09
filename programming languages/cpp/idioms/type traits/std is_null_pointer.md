---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::is_null_pointer` (и переменная-шаблон `std::is_null_pointer_v`) из заголовка `<type_traits>` используется для **проверки, является ли тип `T` — типом `std::nullptr_t`**.

> ✅ Это полезно при шаблонном программировании, когда нужно различать `nullptr`, указатели и другие типы.

```cpp
#include <iostream>
#include <type_traits>

using namespace std;

void print();
  
template<typename T>
void process(const T&);

int main() {
    print();

    int* p0 = new int{42};
    process(p0);
    process(nullptr);

    return 0;
}

void print() {
    cout
        << boolalpha
        << "is_null_pointer_v<decltype(nullptr)>: "
        << is_null_pointer_v<decltype(nullptr)>
        << endl
        << "is_null_pointer_v<int*>: "
        << is_null_pointer_v<int*>
        << endl
        << "is_null_pointer_v<void>: "
        << is_null_pointer_v<void>
        << endl
        << noboolalpha;
}

template<typename T>
void process(const T& _value) {
    if constexpr (is_null_pointer_v<T>){
        cout << "You passed nullptr" << endl;
    } else {
        cout << "You passed value" << endl;
    }
}
```

```
is_null_pointer_v<decltype(nullptr)>: true
is_null_pointer_v<int*>: false
is_null_pointer_v<void>: false
You passed value
You passed nullptr
```

