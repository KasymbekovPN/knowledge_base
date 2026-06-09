---
tags:
  - programming-language
  - cpp
  - data-types
  - string
---
[[__cpp data types static string__|<==]]

Для считывания введенной строки с консоли, как и для считывания других значений, можно использовать объект _std_::_cin_.

```cpp
#include <iostream>
#include <string>

using std::cout;
using std::cin;
using std::endl;
using std::string;

int main(int argc, char const *argv[]) {
    string line0;
    cout << "Input line0: ";
    getline(cin, line0);
    cout << "line0 <= '" << line0 << "'" << endl;

    string line1;
    cout << "Input line1: ";
    cin >> line1;
    cout << "line1 <= '" << line1 << "'" << endl;

    return 0;
}
```

```
Input line0: abc def
line0 <= 'abc def'
Input line1: abc def
line1 <= 'abc'
```

Если во втором способе ввода строка будет содержать подстроки, разделенные пробелом, то `std::cin` будет использовать только первую подстроку.

---
[Строки](https://metanit.com/cpp/tutorial/2.16.php)