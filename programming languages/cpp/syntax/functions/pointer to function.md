---
tags:
  - programming-language
  - cpp
  - syntax
  - function
  - pointer
---
[[__cpp syntax functions__|<==]]

Указатель на функцию (_function_ _pointer_) хранит адрес функции. По сути указатель на функцию содержит адрес первого байта в памяти, по которому располагается выполняемый код функции.

Самым распространенным указателем на функцию является ее имя. С помощью имени функции можно вызывать ее и получать результат ее работы.

Но также указатель на функцию мы можем определять в виде отдельной переменной с помощью следующего синтаксиса:

```
type (*pointer_name)(parameters);
```

- _type_ представляет тип возвращаемого функцией значения.
- _pointer_name_ представляет произвольно выбранный идентификатор в соответствии с правилами о наименовании переменных.
- _parameters_ определяют типы параметров через запятую (при их наличии).

Указатель может указывать только на такую функцию, которая имеет тот же возвращаемый тип и типы параметров, что и определение указателя на функцию.

```cpp
#include <iostream>

void hello(std::string);
void goodbye(std::string);

int main(int argc, char const *argv[]) {
    void (*message)(std::string);

    message = hello;
    message("H");

    message = goodbye;
    message("G");

    return 0;
}

void hello(std::string level) {
    std::cout << "[hello] " << level << std::endl;
}

void goodbye(std::string level) {
    std::cout << "[goodbye] " << level << std::endl;
}
```

```
[hello] H
[goodbye] G
```

#### Определение и инициализация указателя

При определении указатель можно сразу инициализировать, а можно инициализировать значением _nullptr_

Если указатель при определении инициализируется какой-либо функцией, то можно опустить все определение типа и просто использовать слово _auto_. Можно подчеркнуть, что переменная является именно указателем, указав после _auto_ символ _*_. Но особой разницы - что со звездочкой, что без звездочки нет.

Стоит отметить, что при присвоении функции мы можем применять операцию получения адреса. Но в принципе применение такого символа, как и символа звездочки с _auto_, ни на что не влияет.

```cpp
#include <iostream>

void hello(std::string);

int main(int argc, char const *argv[]) {
    void (*message_0)(std::string) {nullptr};
    message_0 = hello;
    void (*message_1)(std::string) = hello;
    void (*message_2)(std::string) {hello};
    auto message_3 = hello;
    auto message_4 {hello};
    auto* message_5 {hello};
    auto message_6 {&hello};

    message_0("message_0");
    message_1("message_1");
    message_2("message_2");
    message_3("message_3");
    message_4("message_4");
    message_5("message_5");
    message_6("message_6");

    return 0;
}

void hello(std::string key) {
    std::cout << "hello " << key << " !!!" << std::endl;
}
```

```
hello message_0 !!!
hello message_1 !!!
hello message_2 !!!
hello message_3 !!!
hello message_4 !!!
hello message_5 !!!
hello message_6 !!!
```

#### Массивы указателей на функции

Кроме одиночных указателей на функции мы можем определять их массивы. Для этого используется следующий формальный синтаксис.

```
type (*array_name[size]) (parameters);
```

```cpp
#include <iostream>

using std::cout;
using std::endl;

int add(const int*, const int*);
int substract(const int*, const int*);
int multiply(const int*, const int*);

int main(int argc, char const *argv[]) {
    const int first {10};
    const int second {5};

    const size_t SIZE = 3;
    const std::string names[SIZE] {"add", "sub", "multi"};
    int (*operations[SIZE])(const int*, const int*) = {
	    add,
	    substract,
	    multiply};

    for (size_t i {0}; i < SIZE; i++) {
        cout << names[i]
		     << " => " 
		     << operations[i](&first, &second)
		     << endl;
    }
    return 0;
}

int add(const int* p0, const int* p1) {
    return *p0 + *p1;
}

int substract(const int* p0, const int* p1) {
    return *p0 - *p1;
}

int multiply(const int* p0, const int* p1) {
    return *p0 * *p1;
}
```

```
add => 15
sub => 5
multi => 50
```

---
[Указатели на функции](https://metanit.com/cpp/tutorial/4.8.php)