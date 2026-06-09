---
tags:
  - programming-language
  - cpp
  - data-types
  - string
---
[[__static data types index__|<==]]

Мы можем работать со строками в __С++__ в так называемом __С__-стиле как с массивами символов, которые оканчиваются на нулевой байт `\0`. Однако, что если такой символ не будет найден или в процессе манипуляций со строкой будет удален, то дальнейшие действия с такой строкой могут иметь недетерминированный результат. По этой причине строки в __С__-стиле считаются небезопасными, и рекомендуется для хранения строк в __C++__ использовать тип _std_::_string_ из модуля `string`.

Объект типа _string_ содержит последовательность символов типа _char_, которая может быть пустой.

Также можно инициализировать или присвоить переменной _string_ конкретную строку.

```cpp
#include <iostream>
#include <string>

using std::cout;
using std::endl;
using std::string;

int main(int argc, char const *argv[]) {
    string empty_msg;
    string msg0 {"some message !!!"};
    string msg1 {msg0};
    string msg2 (msg0);
    string msg3 = msg0;

    cout << "empty_msg <= '" << empty_msg << "'" << endl;
    cout << "msg0 <= '" << msg0 << "'" << endl;
    cout << "msg1 <= '" << msg1 << "'" << endl;
    cout << "msg2 <= '" << msg2 << "'" << endl;
    cout << "msg3 <= '" << msg3 << "'" << endl;

    return 0;
}
```

```
empty_msg <= ''
msg0 <= 'some message !!!'
msg1 <= 'some message !!!'
msg2 <= 'some message !!!'
msg3 <= 'some message !!!'
```

В данном случае переменные _msg1_, _msg2_, _msg3_ получат копию строкового литерала из _msg0_. В своем внутреннем представлении переменная _msg0_ будет хранить массив символов, который также заканчивается на нулевой байт. Однако реализация типа _string_ и предлагаемые им возможности делают работу с этим типом более безопасной.

[[getting & changing of string's symbols]]
[[reading string from console]]

---
[Строки](https://metanit.com/cpp/tutorial/2.16.php)