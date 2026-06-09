---
tags:
  - programming-language
  - syntax
  - cpp
  - function
---

При использовании функций стоит учитывать, что компилятор должен знать о функции до ее вызова. Поэтому вызов функции должен происходить после ее определения, как в случае выше. В некоторых языках это не имеет значение, но в языке __C++__ это играет большую роль. И если, к примеру, мы сначала вызовем, а потом определим функцию, то мы получим ошибку на этапе компиляции, как в следующем случае.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    hello();

    return 0;
}

void hello() {
    cout << "hello" << endl;
}
```

```
.\simple_function_decl_error.cpp:8:5: error: use of undeclared identifier 'hello'
    8 |     hello();
```

В этом случае перед вызовом функции надо ее дополнительно объявить. Объявление функции еще называют прототипом. Формальное объявление выглядит следующим образом.

```
type function_name(parameters);
```

```cpp
#include <iostream>

using std::cout;
using std::endl;

void hello();

int main(int argc, char const *argv[]) {
    hello();

    return 0;
}

void hello() {
    cout << "hello" << endl;
}
```

```
hello
```

В данном случае несмотря на то, что определение функции идет после ее вызова, но так как функция уже объявлена до ее вызова, то компилятор будет знать о функции _hello_, и никаких проблем в работе программы не возникнет.

---
[Определение и объявление функции](https://metanit.com/cpp/tutorial/3.1.php)