---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/count/_|<=]]

```cpp
#include <iostream>
#include <string>
#include <algorithm>

using namespace std;

int main() {
    const string s {"hello, world !!!"};
    const char target {'l'};

    cout
        << "Number of '" << target << "' => "
        << count(s.begin(), s.end(), target)
        << endl;
  
    return 0;
}
```

```
Number of 'l' => 3
```
