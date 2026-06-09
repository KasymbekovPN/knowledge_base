---
tags:
  - programming-language
  - cpp
  - syntax
  - operation
  - arithmetic-operation
  - priority
---
[[__cpp operators index__|<=]]

| Приоритет | Оператор     | Описание                                                                                                                                   | Ассоциативность                               |
| --------- | ------------ | ------------------------------------------------------------------------------------------------------------------------------------------ | --------------------------------------------- |
| _1_       | __::__       | [[scope resolution]]                                                                                                                       | [[associativity of operators\|Left-to-right]] |
| 2         | __a++__      | [[increment\|Постфиксный инкремент]]                                                                                                       | [[associativity of operators\|Left-to-right]] |
| 2         | __a--__      | [[decrement\|Постфиксный декремент]]                                                                                                       | [[associativity of operators\|Left-to-right]] |
| 2         | __type()__   | [Functional cast](https://en.cppreference.com/w/cpp/language/explicit_cast)                                                                | [[associativity of operators\|Left-to-right]] |
| 2         | __type{}__   | [Functional cast](https://en.cppreference.com/w/cpp/language/explicit_cast)                                                                | [[associativity of operators\|Left-to-right]] |
| 2         | __a()__      | [Оператор вызова функции](https://en.cppreference.com/w/cpp/language/operator_other#Built-in_function_call_operator)                       | [[associativity of operators\|Left-to-right]] |
| 2         | __a[]__      | [Оператор подписки](https://en.cppreference.com/w/cpp/language/operator_member_access#Built-in_subscript_operator)                         | [[associativity of operators\|Left-to-right]] |
| 2         | __.__        | [Доступ к полю](https://en.cppreference.com/w/cpp/language/operator_member_access#Built-in_member_access_operators)                        | [[associativity of operators\|Left-to-right]] |
| 2         | __->__       | [Доступ к полю](https://en.cppreference.com/w/cpp/language/operator_member_access#Built-in_member_access_operators)                        | [[associativity of operators\|Left-to-right]] |
| _3_       | __++a__      | [[increment\|Префиксный инкремент]]                                                                                                        | [[associativity of operators\|Right-to-left]] |
| _3_       | __--a__      | [[decrement\|Префиксный декремент]]                                                                                                        | [[associativity of operators\|Right-to-left]] |
| _3_       | __+a__       | [Унарный плюс](https://en.cppreference.com/w/cpp/language/operator_arithmetic#Unary_arithmetic_operators)                                  | [[associativity of operators\|Right-to-left]] |
| _3_       | __-a__       | [Унарный минус](https://en.cppreference.com/w/cpp/language/operator_arithmetic#Unary_arithmetic_operators)                                 | [[associativity of operators\|Right-to-left]] |
| _3_       | __!__        | [Инверсия](https://en.cppreference.com/w/cpp/language/operator_logical)                                                                    | [[associativity of operators\|Right-to-left]] |
| _3_       | __~__        | [Побитная инверсия](https://en.cppreference.com/w/cpp/language/operator_arithmetic#Bitwise_logic_operators)                                | [[associativity of operators\|Right-to-left]] |
| _3_       | __(type)__   | [C-style cast](https://en.cppreference.com/w/cpp/language/explicit_cast)                                                                   | [[associativity of operators\|Right-to-left]] |
| _3_       | __\*a__      | [Оператор разыменовывание](https://en.cppreference.com/w/cpp/language/operator_member_access#Built-in_indirection_operator)                | [[associativity of operators\|Right-to-left]] |
| _3_       | __&a__       | [Оператор адреса](https://en.cppreference.com/w/cpp/language/operator_member_access#Built-in_address-of_operator)                          | [[associativity of operators\|Right-to-left]] |
| _3_       | __sizeof__   | [sizeof](https://en.cppreference.com/w/cpp/language/sizeof)                                                                                | [[associativity of operators\|Right-to-left]] |
| _3_       | __co_wait__  | [co_wait](https://en.cppreference.com/w/cpp/keyword/co_await)                                                                              | [[associativity of operators\|Right-to-left]] |
| _3_       | __new__      | [new, new[]](https://en.cppreference.com/w/cpp/language/new)                                                                               | [[associativity of operators\|Right-to-left]] |
| _3_       | __delete__   | [delete, delete[]](https://en.cppreference.com/w/cpp/language/delete)                                                                      | [[associativity of operators\|Right-to-left]] |
| 4         | __.*__       | [poiner-to-member operator](https://en.cppreference.com/w/cpp/language/operator_member_access#Built-in_pointer-to-member_access_operators) | [[associativity of operators\|Left-to-right]] |
| 4         | __->*__      | [poiner-to-member operator](https://en.cppreference.com/w/cpp/language/operator_member_access#Built-in_pointer-to-member_access_operators) | [[associativity of operators\|Left-to-right]] |
| _5_       | __a*b__      | [[multiplication]]                                                                                                                         | [[associativity of operators\|Left-to-right]] |
| _5_       | __a/b__      | [[division]]                                                                                                                               | [[associativity of operators\|Left-to-right]] |
| _5_       | __a%b__      | [[modulo]]                                                                                                                                 | [[associativity of operators\|Left-to-right]] |
| 6         | __a+b__      | [[addition]]                                                                                                                               | [[associativity of operators\|Left-to-right]] |
| 6         | __a-b__      | [[substruction]]                                                                                                                           | [[associativity of operators\|Left-to-right]] |
| __7__     | __<<__       | [Побитный сдвиг](https://en.cppreference.com/w/cpp/language/operator_arithmetic#Bitwise_shift_operators)                                   | [[associativity of operators\|Left-to-right]] |
| __7__     | __>>__       | [Побитный сдвиг](https://en.cppreference.com/w/cpp/language/operator_arithmetic#Bitwise_shift_operators)                                   | [[associativity of operators\|Left-to-right]] |
| 8         | __<==>__     | [Three-way comparison operator](https://en.cppreference.com/w/cpp/language/operator_comparison#Three-way_comparison)                       | [[associativity of operators\|Left-to-right]] |
| _9_       | __<__        | [Операторы сравнения](https://en.cppreference.com/w/cpp/language/operator_comparison)                                                      | [[associativity of operators\|Left-to-right]] |
| _9_       | __<=__       | [Операторы сравнения](https://en.cppreference.com/w/cpp/language/operator_comparison)                                                      | [[associativity of operators\|Left-to-right]] |
| _9_       | __>__        | [Операторы сравнения](https://en.cppreference.com/w/cpp/language/operator_comparison)                                                      | [[associativity of operators\|Left-to-right]] |
| _9_       | __>=__       | [Операторы сравнения](https://en.cppreference.com/w/cpp/language/operator_comparison)                                                      | [[associativity of operators\|Left-to-right]] |
| 10        | __\==__      | [Операторы сравнения](https://en.cppreference.com/w/cpp/language/operator_comparison)                                                      | [[associativity of operators\|Left-to-right]] |
| 10        | __!=__       | [Операторы сравнения](https://en.cppreference.com/w/cpp/language/operator_comparison)                                                      | [[associativity of operators\|Left-to-right]] |
| __11__    | __a&b__      | [Побитное И](https://en.cppreference.com/w/cpp/language/operator_arithmetic#Bitwise_logic_operators)                                       | [[associativity of operators\|Left-to-right]] |
| 12        | __a^b__      | [Побитное исключающее ИЛИ](https://en.cppreference.com/w/cpp/language/operator_arithmetic#Bitwise_logic_operators)                         | [[associativity of operators\|Left-to-right]] |
| __13__    | __a\|b__     | [Побитное ИЛИ](https://en.cppreference.com/w/cpp/language/operator_arithmetic#Bitwise_logic_operators)                                     | [[associativity of operators\|Left-to-right]] |
| 14        | __a&&b__     | [Логическое И](https://en.cppreference.com/w/cpp/language/operator_logical)                                                                | [[associativity of operators\|Left-to-right]] |
| __15__    | __a\|\|b__   | [Логическое И](https://en.cppreference.com/w/cpp/language/operator_logical)                                                                | [[associativity of operators\|Left-to-right]] |
| 16        | __a?b:c__    | [Тернарное условие](https://en.cppreference.com/w/cpp/language/operator_other#Conditional_operator)                                        | [[associativity of operators\|Right-to-left]] |
| 16        | __throw__    | [Оператор throw](https://en.cppreference.com/w/cpp/language/throw)                                                                         | [[associativity of operators\|Right-to-left]] |
| 16        | __co_yield__ | [yield-оператор](https://en.cppreference.com/w/cpp/language/coroutines)                                                                    | [[associativity of operators\|Right-to-left]] |
| 16        | __=__        | [Прямое присваивание](https://en.cppreference.com/w/cpp/language/operator_assignment#Builtin_direct_assignment)                            | [[associativity of operators\|Right-to-left]] |
| 16        | __+=__       | [Совмещенное присваивание](https://en.cppreference.com/w/cpp/language/operator_assignment#Builtin_compound_assignment)                     | [[associativity of operators\|Right-to-left]] |
| 16        | __-=__       | [Совмещенное присваивание](https://en.cppreference.com/w/cpp/language/operator_assignment#Builtin_compound_assignment)                     | [[associativity of operators\|Right-to-left]] |
| 16        | __\*=__      | [Совмещенное присваивание](https://en.cppreference.com/w/cpp/language/operator_assignment#Builtin_compound_assignment)                     | [[associativity of operators\|Right-to-left]] |
| 16        | __\\=__      | [Совмещенное присваивание](https://en.cppreference.com/w/cpp/language/operator_assignment#Builtin_compound_assignment)                     | [[associativity of operators\|Right-to-left]] |
| 16        | __%=__       | [Совмещенное присваивание](https://en.cppreference.com/w/cpp/language/operator_assignment#Builtin_compound_assignment)                     | [[associativity of operators\|Right-to-left]] |
| 16        | __<<=__      | [Совмещенное присваивание](https://en.cppreference.com/w/cpp/language/operator_assignment#Builtin_compound_assignment)                     | [[associativity of operators\|Right-to-left]] |
| 16        | __>>=__      | [Совмещенное присваивание](https://en.cppreference.com/w/cpp/language/operator_assignment#Builtin_compound_assignment)                     | [[associativity of operators\|Right-to-left]] |
| 16        | __&=__       | [Совмещенное присваивание](https://en.cppreference.com/w/cpp/language/operator_assignment#Builtin_compound_assignment)                     | [[associativity of operators\|Right-to-left]] |
| 16        | __^=__       | [Совмещенное присваивание](https://en.cppreference.com/w/cpp/language/operator_assignment#Builtin_compound_assignment)                     | [[associativity of operators\|Right-to-left]] |
| 16        | __\|=__      | [Совмещенное присваивание](https://en.cppreference.com/w/cpp/language/operator_assignment#Builtin_compound_assignment)                     | [[associativity of operators\|Right-to-left]] |
| __17__    | __,__        | [Comma](https://en.cppreference.com/w/cpp/language/operator_other#Built-in_comma_operator)                                                 | [[associativity of operators\|Left-to-right]] |

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int a {8};
    int b {7};
    int c {a + 5 * ++b};
    int count {1};
    int result {++count * 3 + count++ * 5};
    int d {8};
    int e {7};
    int f {(a + 5) * ++b};

    cout << "a <= " << a << endl;
    cout << "b <= " << b << endl;
    cout << "c = a + 5 ++b <= " << c << endl;
    cout << endl;
    cout << "count <= " << count << endl;
    cout << "result = ++count * 3 + count++ * 5 <= " << result << endl;
    cout << endl;
    cout << "d <= " << d << endl;
    cout << "e <= " << e << endl;
    cout << "f = (a + 5) * ++b <= " << f << endl;

    return 0;
}
```

```
a <= 8
b <= 9
c = a + 5 ++b <= 48

count <= 3
result = ++count * 3 + count++ * 5 <= 16

d <= 8
e <= 7
f = (a + 5) * ++b <= 117
```

---
[Арифметические операции|metanit.com](https://metanit.com/cpp/tutorial/2.6.php)
[C++ Operator Precedence](https://en.cppreference.com/w/cpp/language/operator_precedence)