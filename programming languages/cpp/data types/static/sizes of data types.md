---
tags:
  - programming-language
  - cpp
  - syntax
  - data-types
  - size
---
[[__static data types index__|<=]]

[Типы данных|metanit.com](https://metanit.com/cpp/tutorial/2.3.php)

При перечислении типов данных указывался размер, который он занимает в памяти. Но стандарт языка устанавливает лишь минимальные значения.

| Type        | Min size |
| ----------- | -------- |
| short       | 16 bit   |
| int         | 16 bit   |
| long        | 32 bit   |
| long double | 64 bit   |
Размеры типов имеют связь:

- __long__ >= __int__
- __int__ >= __short__
- __long double__ >= __double__

_Конкретные значения размеров остаются за разработчиками компиляторов_

Однако бывают ситуации, когда необходимо точно знать размер определенного типа. И для этого в __С++__ есть оператор _sizeof()_, который возвращает размер памяти в байтах, которую занимает переменная:
```cpp
#include <iostream>

int main(int argc, char const *argv[]){
    bool bool_value {false};
    signed char signed_char_value{-64};
    unsigned char unsigned_char_value{64};
    short short_value{-88};
    unsigned short unsigned_short_value{88};
    int int_value{-1024};
    unsigned int unsigned_int_value{1024};
    long long_value{-2048};
    unsigned long unsigned_long_value{2048};
    long long long_long_value{-4096};
    unsigned long long unsigned_long_long_value{4096};
    float float_value{23.45};
    double double_value{1.0};
    long double long_double_value{1.1};

    std::cout << "bool_value: " << sizeof(bool_value) << "\n";
    std::cout << "signed_char_value: " << sizeof(signed_char_value) << "\n";
    std::cout << "unsigned_char_value: " << sizeof(unsigned_char_value) << "\n";
    std::cout << "short_value: " << sizeof(short_value) << "\n";
    std::cout << "unsigned_short_value: " << sizeof(unsigned_short_value) << "\n";
    std::cout << "int_value: " << sizeof(int_value) << "\n";
    std::cout << "unsigned_int_value: " << sizeof(unsigned_int_value) << "\n";
    std::cout << "long_value: " << sizeof(long_value) << "\n";
    std::cout << "unsigned_long_value: " << sizeof(unsigned_long_value) << "\n";
    std::cout << "long_long_value: " << sizeof(long_long_value) << "\n";
    std::cout 
	    << "unsigned_long_long_value: "
	    << sizeof(unsigned_long_long_value)
	    << "\n";
    std::cout << "float_value: " << sizeof(float_value) << "\n";
    std::cout << "double_value: " << sizeof(double_value) << "\n";
    std::cout << "long_double_value: " << sizeof(long_double_value) << "\n";

    return 0;
}
```