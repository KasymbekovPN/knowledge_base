---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::is_class_v<T>` из заголовка `<type_traits>` проверяет, является ли тип **классом или структурой (class/struct)**.

> 🔧 Подключается через:
```cpp
#include <type_traits>
```

```cpp
#include <iostream>
#include <type_traits>

struct Point { double x, y; };

enum class Status { Ok, Error };

class Vec3 {

private:
    double x, y, z;
public:
    Vec3(double _x, double _y, double _z): x{_x}, y{_y}, z{_z} {}
};

template<typename T>
void test(const T&);

int main() {
    test(42);
    test("hello");
    test(Vec3{1, 2, 3});
    test(Point());
    test(Status::Ok);

    return 0;
}

template<typename T>
void test(const T& _input) {
    if constexpr (std::is_class_v<T>) {
        std::cout << "A class type";
    } else {
        std::cout << "Not a class type";
    }
    std::cout << std::endl;
}
```

```
Not a class type
Not a class type
A class type    
A class type    
Not a class type
```
