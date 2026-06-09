---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/min_max/std lexicographical_compare/_|<=]]

```cpp
#include <iostream>
#include <string>
#include <algorithm>

using namespace std;

int main() {
    const string FIRST_STR {"apple"};
    const string SECOND_STR {"APPLE"};

    bool result = lexicographical_compare(
        FIRST_STR.begin(),
        FIRST_STR.end(),
        SECOND_STR.begin(),
        SECOND_STR.end()
    );
    cout << boolalpha << result << endl;

    result = lexicographical_compare(
        FIRST_STR.begin(),
        FIRST_STR.end(),
        SECOND_STR.begin(),
        SECOND_STR.end(),
        [](char a, char b) {
            return tolower(static_cast<unsigned char>(a)) ==
                    tolower(static_cast<unsigned char>(b));
        }
    );
    cout << result << noboolalpha << endl;

    return 0;
}
```

```
false
true
```
