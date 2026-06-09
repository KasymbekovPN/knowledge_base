---
tags:
  - programming-language
  - cpp
  - RAII
---
[[RAII|<=]]

**Даже если произойдёт исключение — файл будет корректно закрыт.**

```cpp
#include <fstream>
#include <iostream>

void write_to_file();

int main() {
    write_to_file();

    return 0;
}

void write_to_file() {
    std::ofstream file("data.txt");
    file << "Hello, world !!!" << std::endl;
    
    // code, may be exception
    
} // <- descructor will close file automatically
```
