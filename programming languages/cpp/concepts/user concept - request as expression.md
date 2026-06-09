---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/_|<=]]

```cpp
#include <iostream>
#include <concepts>

template<typename T>
T test(T, T)
requires (std::integral<T> || std::floating_point<T>);

int main(int argc, char const *argv[]) {
    std::cout << (test(36, 12)) << std::endl;
    std::cout << test(3.0f, 12.0f) << std::endl;
    // std::cout << test("3.0f", "12.0f") << std::endl; // Error

    return 0;
}

template<typename T>
T test(T a, T b)
requires (std::integral<T> || std::floating_point<T>) {
    return b != 0 ? a / b : 0;
}
```

```
.\as_expression.cpp:11:18: error: no matching function for call to 'test'
   11 |     std::cout << test("3.0f", "12.0f") << std::endl;
      |                  ^~~~
.\as_expression.cpp:5:3: note: candidate template ignored: constraints not satisfied [with T = const char *]
    5 | T test(T, T)
      |   ^
.\as_expression.cpp:6:11: note: because 'const char *' does not satisfy 'integral'
    6 | requires (std::integral<T> || std::floating_point<T>);
      |           ^
C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.35.32215\include\concepts:81:20: note: because 'is_integral_v<const char *>' evaluated to false
   81 | concept integral = is_integral_v<_Ty>;
      |                    ^
.\as_expression.cpp:6:31: note: and 'const char *' does not satisfy 'floating_point'
    6 | requires (std::integral<T> || std::floating_point<T>);
      |                               ^
C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.35.32215\include\concepts:90:26: note: because 'is_floating_point_v<const char *>' evaluated to false
   90 | concept floating_point = is_floating_point_v<_Ty>;
```
