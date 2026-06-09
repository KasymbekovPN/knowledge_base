---
tags:
  - programming-language
  - syntax
  - namespace
  - alias-definition
---
[[__cpp syntax namespaces__|<=]]

[using. Подключение пространств имен и определение псевдонимов|metanit.com](https://metanit.com/cpp/tutorial/2.11.php)

Ключевое слово __using__ также позволяет определять псевдонимы для типов. Это может пригодиться, когда мы работаем с типами с длинными названиями, а определение коротких псевдонимов позволит сократить код. Например:
```cpp
#include <iostream>

using ullong = unsigned long long;

int main(int argc, char const *argv[]) {
    ullong n {123454};

    std::cout << "n <= " << n << std::endl;

    return 0;
}
```

В данном случае для типа _unsigned long long_ определен псевдоним _ullong_. Стоит отметить, что это именно определение псевдонима, а __НЕ определение нового типа__.

Также для определения псевдонимов в __С++__ также может использоваться старый подход в стиле языка __С__ с помощью оператора __typedef__:
```cpp
#include <iostream>

typedef unsigned long long ullong;

int main(int argc, char const *argv[]) {
    ullong n {12345};

    std::cout << "n <= " << n << std::endl;

    return 0;
}
```