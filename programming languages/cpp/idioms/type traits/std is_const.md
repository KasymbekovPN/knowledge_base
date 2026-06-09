---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::is_const_v<T>` из заголовка `<type_traits>` проверяет, является ли **сам тип `T`** квалифицированным как `const`.

> 🔧 Подключается через:
```cpp
#include <type_traits>
```

### Что делает `std::is_const_v<T>`?

Возвращает `true`, если тип **непосредственно объявлен как `const`**, например:

```cpp
const int        → std::is_const_v<const int> == true
int              → std::is_const_v<int> == false
const char*      → std::is_const_v<const char*> == true  // const указатель
char* const      → std::is_const_v<char* const> == true // const указатель (на непостоянные данные)
const char* const → std::is_const_v<const char* const> == true
```

⚠️ Важно:  
- `std::is_const_v<const int>` → `true`
- `std::is_const_v<int>` → `false`
- `std::is_const_v<const int&>` → `false` — потому что ссылка сама по себе не бывает `const` в этом контексте (это часть декларатора)

```cpp
#include <iostream>
#include <type_traits>

template<typename T>
void test();

int main() {
    test<int>();
    test<const int>();
    test<int&>();
    test<const int&>();

    return 0;
}

template<typename T>
void test() {
    if constexpr (std::is_const_v<T>) {
        std::cout << "A const";
    } else if(std::is_reference_v<T> &&
              std::is_const_v<std::remove_reference_t<T>>) {
        std::cout << "A const reference";
    } else {
        std::cout << "Not a const";
    }
    std::cout << std::endl;
}
```

```
Not a const
A const
Not a const
A const reference
```
