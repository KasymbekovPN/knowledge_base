---
tags:
  - programming-language
  - cpp
  - syntax
  - function
---
[[__cpp syntax functions__|<==]]

Функцию _main_ можно определять с параметрами

```cpp
int main(int argc, char* argv[]) {
    // ...
}
```

Первый параметр, _argc_, представляет тип _int_ и хранит количество аргументов командной строки. Второй параметр, _argv[]_, представляет собой массив указателей и хранит все переданные аргументы командной строки в виде строк. 

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    for (size_t i {}; i < argc; i++) {
        cout << "[" << i << "] " << argv[i] << endl;
    }
    cout << "Done." << endl;

    return 0;
}
```

```
.\main_func_args.exe hello world 123
```

```
[0] C:\Users\KasymbekovPN\YandexDisk\projects\studying-cpp\functions\main_func_args.exe
[1] hello
[2] world
[3] 123
Done.
```

---
[Параметры функции main](https://metanit.com/cpp/tutorial/3.10.php)