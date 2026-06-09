---
tags:
  - programming-language
  - cpp
  - syntax
  - array
  - symbol
---
[[__cpp syntax arrays index__|<==]]

Свои особенности имеют символьные массивы. При инициализации мы можем передать символьному массиву как набор символов, так и строку.

На первый взгляд оба массива будут иметь один и тот же набор символов, пусть в первом случае это просто набор отдельных символов, а во втором - строка. Но в первом случае - массив будет иметь _N_ элементов. А во втором случае массив будет иметь _N+1_, поскольку при инициализации строкой в символьный массив автоматически добавляется нулевой символ `\0`.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    const char hello0[] {'h', 'e', 'l', 'l', 'o'};
    const char hello1[] {'h', 'e', 'l', 'l', 'o', '\0'};
    const char hello2[] {"hello"};

    cout << "hello0 <= " << hello0 << endl;
    cout << "hello1 <= " << hello1 << endl;
    cout << "hello2 <= " << hello2 << endl;

    return 0;
}
```

```
hello0 <= hello☺
hello1 <= hello
hello2 <= hello
```

В первом случае консольный вывод не детерминирован, поскольку символьный массив не заканчивается нулевым символом.

#### Ввод
```cpp
#include <iostream>

using std::cout;
using std::cin;
using std::endl;

int main(int argc, char const *argv[]) {
    const int MAX_LEN {64};
    char text[MAX_LEN] {};

    cout << "Enter text:" << endl;
    cin.getline(text, MAX_LEN);

    cout << "Input: " << text << endl;

    return 0;
}
```

```
Enter text:
Hello
Input: Hello
```

Функция _getline()_ потока _cin_ считывает последовательность символов, включая пробелы. По умолчанию, ввод заканчивается, когда считывается символ перевода строки '\n'.

Другая форма функции _getline()_ также принимает третий параметр - символ, который будет выступать сигналом завершения ввода.

---
[Массив символов](https://metanit.com/cpp/tutorial/2.19.php)