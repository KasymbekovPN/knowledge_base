---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::is_integral_v<T>` из заголовка `<type_traits>` проверяет, является ли тип `T` **целочисленным типом**.

К `std::is_integral` относятся:

✅ Да (вернёт `true`):
- `bool`, `char`, `signed char`, `unsigned char`
- `short`, `int`, `long`, `long long`
- Их беззнаковые версии: `unsigned int`, `unsigned long` и т.д.
- Перечисления (`enum`) — в некоторых реализациях, но не всегда напрямую (лучше использовать `std::underlying_type`)

❌ Нет (вернёт `false`):
- `float`, `double`, `long double`
- Указатели (`int*`)
- Классы, структуры
- Массивы, функции

```cpp
#include <iostream>
#include <type_traits>

using namespace std;

template<typename T>
void test(const T&);

int main(){
    test(42);
    test(new int{42});
    test(true);
    test(std::string{"hello"});

    return 0;
}

template<typename T>
void test(const T& _value) {
    if constexpr (is_integral_v<T>) {
        cout << "Integer-like: " << _value << endl;
    } else {
        cout << "Other: " << _value << endl;
    }
}
```

```
Integer-like: 42
Other: 0000021CE65EF7B0
Integer-like: 1
Other: hello
```

