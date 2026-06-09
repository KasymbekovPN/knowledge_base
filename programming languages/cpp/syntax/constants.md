---
tags:
  - programming-language
  - cpp
  - syntax
  - constants
---
[[_cpp syntax|<=]]

[Константы|metanit.com](https://metanit.com/cpp/tutorial/2.5.php)

Кроме переменных в языке программирования __C++__ можно определять константы. Их значение устанавливается один раз и впоследствии мы его не можем изменить. 

Константа определяется практически так же, как и переменная за тем исключением, что в начале определения константы идет ключевое слово _const_. Например:
```cpp
#include <iostream>

  

int main() {
    const int i0 = 123;
    const int i1 {456};
    const int i2 {i0};
    const int i3 {789};
    // const int i4; // compile error

    std::cout << "i0 <= " << i0 << "\n";
    std::cout << "i1 <= " << i1 << "\n";
    std::cout << "i2 <= " << i2 << "\n";
    std::cout << "i3 <= " << i3 << "\n";

    // i3 = 0; // compile error

    return 0;
}
```

И также в процессе программы мы сможем обращаться к значению константы, например,
```cpp
std::cout << "i0 <= " << i0 << "\n";
```

Но если же мы захотим после определения константы присвоить ей некоторое значение, то компилятор не сможет скомпилировать программу и выведет ошибку:
```
.\main.cpp:13:8: error: cannot assign to variable 'i3' with const-qualified type 'const int'
   14 |     i3 = 0;
      |     ~~ ^
.\main.cpp:7:15: note: variable 'i3' declared const here
    7 |     const int i3 {789};
      |     ~~~~~~~~~~^~~~~~~~
```

Если константа не будет инициализирована, то компилятор также выведет ошибку и не сможет скомпилировать программу, как в следующем случае:
```
.\main.cpp:8:15: error: default initialization of an object of const type 'const int'
    8 |     const int i4;
```

В качестве значения константам можно передавать как обычные литералы, так и динамически вычисляемые значения, например, значения переменных или других констант:
```cpp
    const int i0 = 123;
    const int i2 {i0};
```
