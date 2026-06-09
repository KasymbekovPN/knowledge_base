---
tags:
  - programming-language
  - cpp
  - data-types
  - static
  - cast
  - conversion
---
[[__cpp data types index__|<=]]

Для выполнения явных преобразований типов (explicit type conversion) применяется оператор _static_cast_

`static_cast <type>(value)`

Данный оператор преобразует значение в круглых скобках - _value_ к типу, который указан в угловых скобках - _type_. Слово _static_ в названии оператора отражает тот факт, что приведение проверяется статически, то есть во время компиляции.

Применение оператора _static_cast_ указывает компилятору, что мы уверены, что в этом месте надо применить преобразование, поэтому даже при инициализации в фигурных скобках компилятор не сгенерирует ошибку.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    double sum {100.2};
    unsigned int hours {8};
    unsigned int perHour {static_cast<unsigned int>(sum / hours)};
    unsigned int perHour1 {(unsigned int) sum / hours};
    cout 
	    << "static_cast<unsigned int>(" 
	    << sum 
	    << " / " 
	    << hours
	    << ") => "
	    << perHour
	    << endl;
    cout 
	    << "(unsigned int) "
	    << sum
	    << " / "
	    << hours
	    << " => "
	    << perHour1
	    << endl;

    return 0;
}
```

Здесь выражение `static_cast<unsigned int>(sum/hours)` вычисляет значение выражения `sum/hours` (а оно будет представлять тип _double_), и затем преобразует его в тип _unsigned int_

Стоит отметить, что раньше во времена динозавров в С++ применялась операция преобразования, унаследованная от языка Си:

`(type) value`

То есть перед преобразуемым значением в круглых скобках указывался тип, в который надо выполнить преобразование. 

Результат будет тот же. Однако в современном C++ эту операцию практически вытеснил оператор static_cast.

---
[Статическая типизация и преобразования типов|metanit.com](https://metanit.com/cpp/tutorial/2.4.php)