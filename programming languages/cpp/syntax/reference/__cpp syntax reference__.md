---
tags:
  - programming-language
  - cpp
  - syntax
  - reference
---
[[_cpp syntax|<==]]

__Ссылка__ (__reference__) представляет способ манипулировать каким-либо объектом. Фактически __ссылка__ - это альтернативное имя для объекта. Для определения ссылки применяется знак амперсанда _&_.

```cpp
int number {5};
int &ref_number {number};
```

__Cсылка__ _ref_number_ ссылается на объект _number_. При этом в определении ссылки используется тот же тип, который представляет объект, на который ссылка ссылается, то есть в данном случае _int_.

Нет возможность определить __ссылку__, которая ни на что не ссылается.
```cpp
int &ref_number;
```

Нельзя присвоить __ссылке__ литеральное значение, например, число.
```cpp
int &ref_number = 10;
```

После установления __ссылки__ мы можем через нее манипулировать самим объектом, на который она ссылается.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int number {10};
    int &ref_number {number};

    cout
	    << "number = " << number
	    << " | ref_number = " << ref_number << endl;

    ref_number = 11;
    cout 
	    << "number = " << number
	    << " | ref_number = " << ref_number << endl;

    return 0;
}
```

```
number = 10 | ref_number = 10
number = 11 | ref_number = 11
```

Изменения по __ссылке__ неизбежно скажутся и на том объекте, на который ссылается ссылка.

Можно определять не только __ссылки__ на переменные, но и __ссылки__ на константы, но при этом ссылка сама должна быть константной.
```cpp
const int number {5};
const int &ref_number {5};
```

__Нельзя__ инициализировать не константную __ссылку__ константным объектом.
```cpp
const int number {5};
int &ref_number {number}; // Error
```

Константная __ссылка__ может указывать и на обычную переменную, только значение по такой __ссылке__ мы не сможем изменить.

```cpp
#include <iostream>

int main(int argc, char const *argv[]) {
    int number {5};
    const int &ref {number};

    ref = 111; // Error

    return 0;
}
```

```
error: cannot assign to variable 'ref' with const-qualified type 'const int &'
```

---
[Ссылки](https://metanit.com/cpp/tutorial/2.14.php)