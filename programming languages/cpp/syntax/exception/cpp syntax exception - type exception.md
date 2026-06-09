---
tags:
  - programming-language
  - cpp
  - syntax
  - exception
---
[[_cpp syntax - exception|<=]]

Все исключения в языке C++ описываются типом _exception_, который определен в заголовочном файле `<exception>`. 

```cpp
#include <iostream>
#include <exception>

double divide(double, double);

int main(int argc, char const *argv[]){
    try {
        double result {divide(100.0, 0.0)};
        std::cout << "result <= " << result << std::endl;
    } catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    std::cout << "DONE!" << std::endl;

    return 0;
}

double divide(double a, double b) {
    if (!b) {
        throw std::exception();
    }

    return a / b;
}
```

```
Unknown exception
DONE!
```

---
[Тип exception](https://metanit.com/cpp/tutorial/6.2.php)