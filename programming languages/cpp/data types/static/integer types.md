---
tags:
  - programming-language
  - cpp
  - syntax
  - data-types
  - integer
  - char
  - signed-char
  - unsigned-char
  - short
  - unsigned-short
  - install
  - unsigned-int
  - long
  - unsigned-long
  - long-long
  - unsigned-long-long
---
[[__static data types index__|<=]]

[Типы данных|metanit.com](https://metanit.com/cpp/tutorial/2.3.php)
### Целочисленные типы

Целые числа в языке __C++__ представлены следующими типами:

- _signed char_: представляет один символ. Занимает в памяти _1 байт (8 бит)_. Может хранить любой значение из диапазона от _-128_ до _127_

- _unsigned char_: представляет один символ. Занимает в памяти _1 байт (8 бит)_. Может хранить любой значение из диапазона от _0_ до _255_

- _char_: представляет один символ в кодировке __ASCII__. Занимает в памяти _1 байт (8 бит)_. Может хранить любое значение из диапазона от _-128_ до _127_, либо от _0_ до _255_. Несмотря на то, что данный тип представляет тот же диапазон значений, что и вышеописанный тип _signed char_, но они не эквивалентны. Тип _char_ предназначен для хранения числового кода символа и в реальности может представлять как _signed char_, так и _unsigned char_ в зависимости от конкретного компилятора.

- _short_: представляет целое число в диапазоне от _–32768_ до _32767_. Занимает в памяти __2 байта (16 бит)__. Данный тип также имеет псевдонимы _short int_, _signed short int_, _signed short_.

- _unsigned short_: представляет целое число в диапазоне от _0_ до _65535_. Занимает в памяти __2 байта (16 бит)__. Данный тип также имеет синоним _unsigned short int_.

- _int_: представляет целое число. В зависимости от архитектуры процессора может занимать __2 байта (16 бит)__ или __4 байта (32 бита)__. Диапазон предельных значений соответственно также может варьироваться от __–32768__ до __32767__ (при 2 байтах) или от __−2 147 483 648__ до __2 147 483 647__ (при 4 байтах). Но в любом случае размер должен быть больше или равен размеру типа __short__ и меньше или равен размеру типа __long__. Данный тип имеет псевдонимы _signed int_ и _signed_.

- _unsigned int_: представляет положительное целое число. В зависимости от архитектуры процессора может занимать __2 байта (16 бит)__ или __4 байта (32 бита)__, и из-за этого диапазон предельных значений может меняться: от __0__ до __65535__ (для 2 байт), либо от __0__ до __4 294 967 295__ (для 4 байт). Имеет псевдоним _unsigned_.

- _long_: в зависимости от архитектуры может занимать __4__ или __8__ байт и представляет целое число в диапазоне от __−2 147 483 648__ до __2 147 483 647__ (при 4 байтах) или от __−9 223 372 036 854 775 808__ до __+9 223 372 036 854 775 807__ (при 8 байтах). Занимает в памяти 4 байта (32 бита) или 8 байт. Имеет псевдонимы __long int__, __signed long int__ и __signed long__

- _unsigned long_: представляет целое число в диапазоне от __0__ до __4 294 967 295__. Занимает в памяти __4 байта (32 бита)__. Имеет синоним __unsigned long int__.

- _long long_: представляет целое число в диапазоне от _−9 223 372 036 854 775 808_ до _+9 223 372 036 854 775 807_. Занимает в памяти __8 байт (64 бита)__. Имеет псевдонимы _long long int_, _signed long long int_ и _signed long long_.

- _unsigned long long_: представляет целое число в диапазоне от __0__ до __18 446 744 073 709 551 615__. Занимает в памяти, как правило, __8 байт (64 бита)__. Имеет псевдоним _unsigned long long int_.

Для представления чисел в __С++__ применятся целочисленные литералы со знаком или без, типа -10 или 10.

```cpp
#include <iostream>

int main(int argc, char const *argv[]){
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

    std::cout << "signed_char_value: " << signed_char_value << "\n";
    std::cout << "unsigned_char_value: " << unsigned_char_value << "\n";
    std::cout << "short_value: " << short_value << "\n";
    std::cout << "unsigned_short_value: " << unsigned_short_value << "\n";
    std::cout << "int_value: " << int_value << "\n";
    std::cout << "unsigned_int_value: " << unsigned_int_value << "\n";
    std::cout << "long_value: " << long_value << "\n";
    std::cout << "unsigned_long_value: " << unsigned_long_value << "\n";
    std::cout << "long_long_value: " << long_long_value << "\n";
    std::cout << "unsigned_long_long_value: " << unsigned_long_long_value << "\n";

    return 0;
}
```

Все целочисленные литералы по умолчанию представляют тип _int_. Так, выше переменным разных типов присваивались различные числа - 64, -64, 88, -88, 1024 и т.д. Но все эти целочисленные литералы представляют тип _int_.

Могут быть использщованы целочисленные литералы и других типов. Целочисленные литералы без знака (которые представляют _unsigned_-типы) имеют суффикс __u__ или __U__. Литералы типов _long_ и _long long_ имеют суффиксы __L/l__ и __LL/ll__ соответственно:
```cpp
#include <iostream>

int main(int argc, char const *argv[])
{
    unsigned int unsigned_int_value{1024U};
    long long_value{-2048L};
    unsigned long unsigned_long_value{20248UL};
    long long long_long_value{-4096LL};
    unsigned long long unsigned_long_long_value{4096ULL};

    std::cout << "unsigned_int_value: " << unsigned_int_value << "\n";
    std::cout << "long_value: " << long_value << "\n";
    std::cout << "unsigned_long_value: " << unsigned_long_value << "\n";
    std::cout << "long_long_value: " << long_long_value << "\n";
    std::cout << "unsigned_long_long_value: " << unsigned_long_long_value << "\n";

    return 0;
}
```

Использовать суффиксы необязательно, поскольку, как правило, компилятор может успешно преобразовать целочисленный литерал типа (который технически представляет тип __int__) к нужному типу без потери информации.

Если число большое, то при вводе мы можем где-то ошибиться. Чтобы упростить читабельность чисел, начиная со стандарта __C++14__ в язык была добавлена возможность разделения разрядов числа с помощью одинарной кавычки __'__
```cpp
#include <iostream>

int main(int argc, char const *argv[]){
    int value{1'234'567'890};
    std::cout << "value <= " << value << "\n";

    return 0;
}
```

#### Различные системы исчисления

По умолчанию все стандартные целочисленные литералы представляют числа в привычной нам десятичной системе. Однако __C++__ также позволяет использовать и числа в других системах исчисления.

Система исчисления определяется в литерале префиксом:
- __16__ - __0x__ или __0X__
- __8__ - __0__ 
- __2__ - __0b__ или __0B__
```cpp
#include <iostream>

int main(int argc, char const *argv[]){
    int hex_int_value{0x1A};
    int oct_int_value{034};
    int bin_int_value{0b1010};

    std::cout << "hex_int_value: " << hex_int_value << "\n";
    std::cout << "oct_int_value: " << oct_int_value << "\n";
    std::cout << "bin_int_value: " << bin_int_value << "\n";

    return 0;
}
```

Все эти типы литералов также поддерживают суффиксы __U/L/LL__