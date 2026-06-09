---
tags:
  - programming-language
  - cpp
  - syntax
  - function
  - pointer
---
[[__cpp syntax functions__|<==]]

Параметры функции в __C++__ могут представлять указатели. Указатели передаются в функцию по значению, то есть функция получает копию указателя. В то же время копия указателя будет в качестве значения иметь тот же адрес, что оригинальный указатель. Поэтому используя в качестве параметров указатели, мы можем получить доступ к значению аргумента и изменить его.

```cpp
#include <iostream>

using std::cout;
using std::endl;

void func(int*, bool swap);

int main(int argc, char const *argv[]) {
    int number {42};
    int* pnumber {&number};

    func(pnumber, false);
    cout << "*pnumber <= " << *pnumber << endl;

    func(pnumber, true);
    cout << "*pnumber <= " << *pnumber << endl;

    return 0;
}

void func(int* ptr, bool swap) {
    cout << ptr << " | " << *ptr << endl;
    (*ptr)++;
    if (swap) {
        int new_value {111};
        ptr = &new_value;
    }
    cout << ptr << " | " << *ptr << endl;
}
```

```
0000006E01AFF7F0 | 42
0000006E01AFF7F0 | 43
*pnumber <= 43
0000006E01AFF7F0 | 43
0000006E01AFF7A4 | 111
*pnumber <= 44
```

Т.к. аргумент (указатель) передается как копия, то указатель в области видимости _main_ не меняется.

#### Константные параметры-указатели

Параметры, которые представляют указатели, могут быть константными.

По константному параметру мы не можем изменить значение. То есть фактически такие параметры представляют указатели на константу. Поэтому константные параметры полезны, когда необходимо передать в функцию адрес константы - в этом случае параметр обязательно должен быть константным:

При этом константность параметра не означает, что мы не можем изменить адрес, хранимый в указателе.

```cpp
#include <iostream>

using std::cout;
using std::endl;

void func(const int*);
void func_with_swap(const int*);
void func_with_error(const int*);

int main(int argc, char const *argv[]) {
    int number {42};
    const int cnumber {142};

    int* pnumber {&number};
    const int* pcnumber {&cnumber};

    func(pnumber);
    func(pcnumber);

    func_with_swap(pnumber);
    func_with_swap(pcnumber);
    cout << "*pnumber <= " << *pnumber << endl;
    cout << "*pcnumber <= " << *pcnumber << endl;

    // func_with_error(pnumber);

    return 0;
}

void func(const int* ptr) {
    cout << "[func] " << *ptr << endl;
}

void func_with_swap(const int* ptr) {
    int new_value = 123;
    ptr = &new_value;
    cout << "[func_with_swap] " << *ptr << endl;
}

// void func_with_error(const int* ptr) {
//     *ptr = 456; // Error
// }
```

```
[func] 42
[func] 142
[func_with_swap] 123
[func_with_swap] 123
*pnumber <= 42
*pcnumber <= 142
```

```
.\const_pointer_param.cpp:41:10: error: read-only variable is not assignable
   41 |     *ptr = 456;
      |     ~~~~ ^
```

Чтобы гарантировать, что не только значение по указателю не будет меняться, но и само значение указателя (хранимый в нем адрес) не будет меняться, надо определить указатель как константный.

```cpp
#include <iostream>

using std::cout;
using std::endl;

void func(const int* const);

int main(int argc, char const *argv[]) {
    int number {42};
    func(&number);

    return 0;
}

void func(const int* const ptr) {
    int new_value {146};
    // ptr = &new_value; // Error

    cout << "new_value <= " << new_value << endl;
    cout << "*ptr <= " << *ptr << endl;
}
```

```
new_value <= 146
*ptr <= 42
```

```
.\const_const_pointer_param.cpp:17:9: error: cannot assign to variable 'ptr' with const-qualified type 'const int *const'
   17 |     ptr = &new_value;
      |     ~~~ ^
```

#### Параметры по ссылке или параметры-указатели

Параметры, передаваемые по ссылке, и параметры-указатели похожи в том плане, что оба эти вида параметров позволяют менять значения передаваемых в них переменных. Единственной отличительной особенностью указателя является то, что он может иметь значение _nullptr_, в то время как ссылка всегда должна ссылаться на что-то. Поэтому, если необходимо, что параметр не имел никакого значения, то можно использовать указатели. Единственное, что в этом случае необходимо проверять указатель на значение _nullptr_ перед его использованием.

---
[Указатели в параметрах функции](https://metanit.com/cpp/tutorial/4.6.php)