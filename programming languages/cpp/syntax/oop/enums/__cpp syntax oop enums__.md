---
tags:
  - programming-language
  - cpp
  - syntax
  - enum
---
[[__cpp syntax oop__|<==]]

__Перечисления__ (_enum_) представляют еще один способ определения своих типов. Их отличительной особенностью является то, что они содержат набор числовых констант.

```cpp
enum class enum_name {
	const_0,
	const_1,
	// ...
	const_N
};
```

После ключевых _enum_ _class_ идет название перечисления, и затем в фигурных скобках перечисляются через запятую константы перечисления.

```cpp
#include <iostream>

enum class EnumExample {
    Zero,
    First,
    Second
};

enum class Day {
    Monday = 2,
    Tuesday,
    Wednesday,
    Thursday,
    Friday,
    Saturday,
    Sunday = 1
};

enum class Operation {
    Add = '+',
    Substract = '-',
    Multiply = '*'
};

int main(int argc, char const *argv[]) {
    std::cout
	    << "EnumnExample::Zero <= "
	    << (int) EnumExample::Zero << std::endl;
    std::cout
	    << "EnumnExample::First <= "
	    << (int) EnumExample::First << std::endl;
    std::cout
	    << "EnumnExample::Second <= "
	    << (int) EnumExample::Second << std::endl << std::endl;

    std::cout
	    << "Day::Monday <= "
	    << (int) Day::Monday << std::endl;
    std::cout
	    << "Day::Tuesday <= "
	    << (int) Day::Tuesday << std::endl;
    std::cout
	    << "Day::Wednesday <= "
	    << (int) Day::Wednesday << std::endl;
    std::cout
	    << "Day::Thursday <= "
	    << (int) Day::Thursday << std::endl;
    std::cout
	    << "Day::Friday <= "
	    << (int) Day::Friday << std::endl;
    std::cout
	    << "Day::Saturday <= "
	    << (int) Day::Saturday << std::endl;
    std::cout
	    << "Day::Sunday <= "
	    << (int) Day::Sunday << std::endl << std::endl;

    std::cout
	    << "Operation::Add <= '"
	    << static_cast<char>(Operation::Add) << "'" << std::endl;
    std::cout
	    << "Operation::Substract <= '"
	    << static_cast<char>(Operation::Substract) << "'" << std::endl;
    std::cout
	    << "Operation::Multiply <= '"
	    << static_cast<char>(Operation::Multiply) << "'" << std::endl;

    return 0;
}
```

```
EnumnExample::Zero <= 0
EnumnExample::First <= 1
EnumnExample::Second <= 2

Day::Monday <= 2
Day::Tuesday <= 3
Day::Wednesday <= 4
Day::Thursday <= 5
Day::Friday <= 6
Day::Saturday <= 7
Day::Sunday <= 1

Operation::Add <= '+'
Operation::Substract <= '-'
Operation::Multiply <= '*'
```

Каждой константе сопоставляется некоторое числовое значение. По умолчанию первая константа получает в качестве значения 0, а остальные увеличиваются на единицу (_EnumExample_).

Мы также можем управлять установкой значений в перечислении (_Day_).

Стоит учитывать, что константы перечисления должны представлять целочисленные константы. Однако мы можем выбрать другой целочисленный тип, например, _char_ (_Operation_).

[[enum applying]]
[[connecting enumeration constants]]
[[cstyle enums]]

---
[Перечисления](https://metanit.com/cpp/tutorial/5.9.php)