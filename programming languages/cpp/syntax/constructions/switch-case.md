---
tags:
  - programming-language
  - cpp
  - syntax
  - construction
  - switch
  - case
---
[[__cpp syntax constractions__|<=]]

#### switch

Конструкция _switch_-_case_ позволяет сравнить некоторое выражение с набором значений. Она имеет следующую форму

```
switch(condition) {
	case value_0: instruction_0;
	case value_1: instruction_1;
	...
	default: instruction;
}
```

После ключевого слова _switch_ в скобках идет сравниваемое выражение. Значение этого выражения последовательно сравнивается со значениями после оператора _сase_. И если совпадение будет найдено, то будет выполняться определенный блок _сase_.

Стоит отметить, что сравниваемое выражение в _switch_ должно представлять один из целочисленных или символьных типов, или перечисление.

Чтобы избежать выполнения последующих блоков _case_/_default_, в конце каждого блока ставится оператор _break_. То есть в данном случае будет выполняться оператор.

Так же можно определять для нескольких меток _case_ один набор инструкций.

Определение переменных в блоках _case_, возможно, встречается нечасто. Однако может вызвать затруднения. Так, если переменная определяется в блоке _case_, то все инструкции блока помещаются в фигурные скобки (для блока _default_ это не требуется)

В конце конструкции _switch_ может стоять блок _default_. Он необязателен и выполняется в том случае, если значение после _switch_ не соответствует ни одному из операторов case.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int a {1};

    switch (a) {
        case 0:
        case 1: {
            int x{123};
            cout << a << " <> " << x << endl;
            break;
        }

        default: {
            int x{456};
            cout << a << " <> " << x << endl;
            break;
        }
    }

    return 0;
}
```

#### switch с инициализацией переменной

Иногда в конструкции _switch_ для различных промежуточных вычислений необходимо определить переменную. Для этой цели начиная со стандарта __C++17__ язык __С++__ поддерживает особую форму конструкции _switch_.

```
switch(initialization; expression) {
    // ...
}
```

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    char op {'+'};
    int n {2};

    switch (int k{3}; op){
        case '+':
            cout << n << " + "  << k << " = " << n + k << endl;
            break;

        default:
            cout << n << " - "  << k << " = " << n + k << endl;
            break;
    }

    return 0;
}
```

```
2 + 3 = 5
```

---
[Конструкция switch-case](https://metanit.com/cpp/tutorial/2.17.php)