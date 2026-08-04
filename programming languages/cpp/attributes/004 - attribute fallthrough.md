---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/attributes/_|<=]]

`[[fallthrough]]` — атрибут, ставится как отдельный statement в конце `case`-блока внутри `switch`, чтобы явно сообщить компилятору: переход к следующему `case` без `break` сделан намеренно, а не забыт. Подавляет предупреждение вида `-Wimplicit-fallthrough`.

```cmake
cmake_minimum_required(VERSION 3.30)  
project(demo CXX)  
  
add_executable(demo main.cpp)  
target_compile_features(demo PUBLIC cxx_std_23)  
target_compile_options(demo PRIVATE  
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wimplicit-fallthrough>  
)
```

```cpp
#include <iostream>  
  
namespace {  
    void process(const int value) {  
        switch (value) {  
            case 1:  
                std::cout << "one/n";  
                [[fallthrough]];  
            case 2:  
                std::cout << "one or two\n";  
                break;  
            case 3:  
                std::cout << "three\n";  
            case 4:  
                std::cout << "three or four\n";  
                break;  
            default:  
                std::cout << "other\n";  
                break;  
        }    
    }
}  
  
int main() {  
    for (int i{1}; i <= 4; ++i) {  
        process(i);  
    }
}
```

До C++17 для этого использовали комментарии вроде `// fall through` (которые некоторые компиляторы даже умели распознавать эвристически) — атрибут делает это намерение частью языка, а не соглашением.
