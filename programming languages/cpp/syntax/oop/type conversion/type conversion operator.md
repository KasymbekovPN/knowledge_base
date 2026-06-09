---
tags:
  - programming-language
  - cpp
  - syntax
  - operator
  - type-conversion
---
[[__cpp syntax oop type conversion__|<=]]

```cpp
#include <iostream>

class Counter {

private:
    int value;

public:
    Counter(int);
    operator int() const;
    operator bool() const;
};

Counter::Counter(int value): value{value} {}

Counter::operator int() const {
    return value;
}

Counter::operator bool() const {
    return value != 0;
}

void test_as_int(const Counter&, int, std::string);
void test_as_bool(const Counter&, bool, std::string);

int main(int argc, char const *argv[]) {
    Counter counter0 {-1};
    Counter counter1 {0};
    Counter counter2 {1};

    test_as_int(counter0, 0, "counter0 & 0");
    test_as_int(counter0, -1, "counter0 & -1");
    test_as_bool(counter0, true, "counter0 & true");
    test_as_bool(counter0, false, "counter0 & false");
    std::cout << std::endl;

    test_as_int(counter1, 1, "counter0 & 1");
    test_as_int(counter1, 0, "counter0 & 0");
    test_as_bool(counter1, true, "counter1 & true");
    test_as_bool(counter1, false, "counter1 & false");
    std::cout << std::endl;

    test_as_int(counter2, 2, "counter0 & 2");
    test_as_int(counter2, 1, "counter0 & 1");
    test_as_bool(counter2, true, "counter2 & true");
    test_as_bool(counter2, false, "counter2 & false");
    std::cout << std::endl;

    return 0;
}

void test_as_int(const Counter& counter,
				 int expected,
				 std::string message) {
    int gotten_value {static_cast<int>(counter)};
    std::cout
        << "[test_as_int] " << message << ": "
        << std::boolalpha << (gotten_value == expected)
        << std::noboolalpha << std::endl;
}

void test_as_bool(const Counter& counter,
				  bool expected,
				  std::string message) {
    bool gotten_value {static_cast<bool>(counter)};
    std::cout
        << "[test_as_bool] " << message << ": "
        << std::boolalpha << (gotten_value == expected)
        << std::noboolalpha << std::endl;
}
```

```
[test_as_int] counter0 & 0: false
[test_as_int] counter0 & -1: true
[test_as_bool] counter0 & true: true
[test_as_bool] counter0 & false: false

[test_as_int] counter0 & 1: false
[test_as_int] counter0 & 0: true
[test_as_bool] counter1 & true: false
[test_as_bool] counter1 & false: true

[test_as_int] counter0 & 2: false
[test_as_int] counter0 & 1: true
[test_as_bool] counter2 & true: true
[test_as_bool] counter2 & false: false
```

__C++__ позвjляет определить функцию оператора преобразования из типа текущего класса в другой тип. Тип, в который производится преобразование, может быть фундаментальным типом или типом класса. 

```cpp
class ClassName {

public:
	operator OtherType() const;
}
```

В отличие от большинства операторов, операторы преобразования должны быть определены только как функции-члены класса. Их нельзя определить как обычные функции. Они также являются единственными операторами, в которых ключевому слову оператора не предшествует тип возвращаемого значения (вместо этого возвращаемый тип идет после ключевого слова `operator`). 

---
[Операторы преобразования типов](https://metanit.com/cpp/tutorial/5.15.php)