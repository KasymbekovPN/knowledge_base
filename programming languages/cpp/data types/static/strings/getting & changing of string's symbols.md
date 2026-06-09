---
tags:
  - programming-language
  - cpp
  - data-types
  - string
---
[[__cpp data types static string__|<==]]

Подобно массиву мы можем обращаться с помощью индексов к отдельным символам строки, получать и изменять их.

Также поскольку объект _string_ представляет последовательность символов, то эту последовательность можно перебрать с помощью цикла _for_.

```cpp
#include <iostream>
#include <string>

using std::cout;
using std::endl;
using std::string;

int main(int argc, char const *argv[]) {
    string hello {"Hello !!!"};
    cout << "Original hello <= '"  << hello << "'" << endl;

    hello[0] = 'h';
    cout << "Changed hello <= '"  << hello << "'" << endl;

    char first_char = {hello[0]};
    cout << "First char <= '" << first_char << "'" << endl;

    const char FIND_CHAR = 'l';
    unsigned count {};
    for (const char ch : hello) {
        count += ch == FIND_CHAR ? 1 : 0;
    }
    cout << "'l' counter <= " << count << endl;

    return 0;
}
```

```
Original hello <= 'Hello !!!'
Changed hello <= 'hello !!!'
First char <= 'h'
'l' counter <= 2
```

---
[Строки](https://metanit.com/cpp/tutorial/2.16.php)