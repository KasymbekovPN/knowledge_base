---
tags:
  - programming-language
  - cpp
  - syntax
  - function
---
[[__cpp syntax functions__|<==]]

Параметры могут быть константными - значения таких параметров не могут меняться. Константyные параметры предваряются ключевым словом _const_. 

```cpp
#include <iostream>

using std::cout;
using std::endl;

void square(int);

int main(int argc, char const *argv[]) {
    int number {42};
    square(number);

    return 0;
}

void square(const int number) {
    // number = 2; // Error
    int n = number;
    cout << "result <= " << n * n << endl;
}
```

```
result <= 1764
```

```
.\const_function_params.cpp:16:12: error: cannot assign to variable 'number' with const-qualified type 'const int'
   16 |     number = 2;
      |     ~~~~~~ ^
.\const_function_params.cpp:15:23: note: variable 'number' declared const here
   15 | void square(const int number) {
      |             ~~~~~~~~~~^~~~~~
```

Стоит отметить что ключевое слово _const_ для константного параметра используется при определении функции.

При объявлении функции - в ее прототипе для параметров, которые передаются по значению, указывать const необязательно

Для параметров, которые передаются по ссылке, указывать const в прототипе обязательно.

Константному параметру можно передать в качестве аргумента как константу, так и переменную.

#### Константные ссылки

Параметры, которые передаются по ссылке, также могут быть константными.

```cpp
#include <iostream>

using std::cout;
using std::endl;

void square(const int&);

int main(int argc, char const *argv[]) {
    int number {12};
    square(number);

    return 0;
}

void square(const int& number) {
    // number = 1; // Error
    cout << "number^2 <= " << number * number << endl;
}
```

```
number^2 <= 144
```

```
.\const_function_ref_params.cpp:16:12: error: cannot assign to variable 'number' with const-qualified type 'const int &'
   16 |     number = 1; // Error
      |     ~~~~~~ ^
.\const_function_ref_params.cpp:15:24: note: variable 'number' declared const here
   15 | void square(const int& number) {
      |             ~~~~~~~~~~~^~~~~~
```

Значение константной ссылки также нельзя менять.

Если функция получает аргументы по ссылке, то чтобы передать в функцию константу, параметры тоже должны представлять ссылку на константу (не константной ссылке мы не можем передать константу.):

И если в функцию необходимо передать большие объекты, которые не должны изменяться, то определение параметров именно как константных ссылок больше всего подходит для данной задачи.

---
[Константные параметры](https://metanit.com/cpp/tutorial/3.4.php)