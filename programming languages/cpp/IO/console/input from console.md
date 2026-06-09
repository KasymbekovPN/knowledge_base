---
tags:
  - programming-language
  - cpp
  - input
  - console
---
[[__cpp IO console__|<=]]

[Ввод и вывод в консоли|metanit.com](https://metanit.com/cpp/tutorial/2.10.php)

Для считывания с консоли данных применяется оператор ввода __>>__, который принимает два операнда. Левый операнд представляет объект типа _istream_ (в данном случае объект _cin_), с которого производится считывание, а правый операнд - объект, в который считываются данные.
```cpp
#include <iostream>

int main(int argc, char const *argv[]){
    int age;
    int size;
    double weight;

    std::cout << "Input age: ";
    std::cin >> age;

    std::cout << "Input size and weight: ";
    std::cin >> size >> weight;

    std::cout << "Age <= " << age << std::endl;
    std::cout << "Size <= " << size << std::endl;
    std::cout << "Weight <= " << weight << std::endl;

    return 0;
}
```

```
Input age: 123
Input size and weight: 30
30.12
Age <= 123
Size <= 30
Weight <= 30.12
```
