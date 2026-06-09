---
tags:
  - programming-language
  - cpp
  - syntax
  - operator-overloading
  - left-shift
---
[[__cpp syntax oop operator overloading__|<=]]

Оператор __<<__ принимает два аргумента:
- ссылку на объект потока (левый операнд)
- фактическое значение для вывода (правый операнд). 

Затем он возвращает новую ссылку на поток, которую можно передать при следующем вызове оператора __<<__ в цепочке.

Стандартный выходной поток `cout` имеет тип _std::ostream_. Поэтому первый параметр (левый операнд) представляет объект ostream, а второй (правый операнд) - выводимый объект _Counter_. Поскольку мы не можем изменить стандартное определение std::ostream, поэтому определяем функцию оператора, которая не является членом класса.

В данном случае для выводим значение переменной _value_. Для получения значения value извне класса _Counter_ я добавил функцию _getValue()_.

Возвращаемое значение всегда должно быть ссылкой на тот же объект потока, на который ссылается левый операнд оператора.

```cpp
#include <iostream>

class Counter {

private:
    int value;

public:
    Counter(int);
    int getValue() const;
};

Counter::Counter(int value): value{value} {}

int Counter::getValue() const {
    return value;
}

std::ostream& operator<<(std::ostream&, const Counter&);

int main(int argc, char const *argv[]) {
    Counter c0 {42};
    Counter c1 {43};

    std::cout << c0 << std::endl;
    std::cout << c1 << std::endl;

    return 0;
}

std::ostream& operator<<(std::ostream& stream, const Counter& counter) {
    stream << "{value: " << counter.getValue() << "}";
    return stream;
}
```

```
{value: 42}
{value: 43}
```

---
[Перегрузка операторов](https://metanit.com/cpp/tutorial/5.14.php)