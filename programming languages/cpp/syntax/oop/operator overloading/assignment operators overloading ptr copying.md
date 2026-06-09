---
tags:
  - programming-language
  - cpp
  - syntax
  - operator
  - comparison
---
[[__cpp syntax oop operator overloading__|<=]]

```cpp
#include <iostream>

class BaseCounter {

protected:
    int* pointer;

public:
    BaseCounter(int);
    virtual ~BaseCounter();
    virtual void print() const = 0;
};

BaseCounter::BaseCounter(int value) {
    pointer = new int{value};
}

BaseCounter::~BaseCounter() {
    delete pointer;
}

class SimpleCounter: public BaseCounter{

public:
    SimpleCounter(int);
    void print() const override;
};

SimpleCounter::SimpleCounter(int value): BaseCounter{value} {}

void SimpleCounter::print() const {
    std::cout
        << "[simple counter] "
        << *pointer << std::endl;
}

class Counter: public BaseCounter{

public:
    Counter(int);
    void print() const override;
    Counter& operator=(const Counter&);
};

Counter::Counter(int value): BaseCounter{value} {}

void Counter::print() const {
    std::cout
        << "[counter] "
        << *pointer << std::endl;
}

Counter& Counter::operator=(const Counter& other) {
    if (&other != this) {
        *pointer = *other.pointer;
    }
    return *this;
}

int main(int argc, char const *argv[]) {
    std::cout << "### First ###" << std::endl;
    SimpleCounter sc0 {42};
    {
        SimpleCounter sc1 {43};
        sc0 = sc1;
        sc0.print();
    }
    sc0.print();

    std::cout << "### Second ###" << std::endl;
    Counter c0 {99};
    {
        Counter c1 {100};
        c0 = c1;
        c0.print();
    }
    c0.print();

    return 0;
}
```
```
### First ###
[simple counter] 43
[simple counter] -847647072
### Second ###
[counter] 100
[counter] 100
```

Здесь оба экземпляра _SimpleCounter_ содержат один и то же указатель. Удаление _sc1_ приведет к некорректной работе.


### Удаление оператора присваивания

В предыдущей ситуации есть альтернатива реализации оператора присвоения - удаление оператора присвоения по умолчанию. Для этого применяется ключевое слово delete:

|   |   |
|---|---|
|1|`Counter& operator=(``const` `Counter& counter) =` `delete``;`|

В этом случае, если в программе будет применяться операция присвоения, типа

|   |   |
|---|---|
|1|`counter1 = counter2;`|

То компилятор сгенерирует ошибку.


---



---
[Перегрузка операторов](https://metanit.com/cpp/tutorial/5.14.php)
[Переопределение оператора присваивания](https://metanit.com/cpp/tutorial/5.21.php)