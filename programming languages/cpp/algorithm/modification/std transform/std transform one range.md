---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/modification/std transform/_|<=]]

```cpp
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>

using namespace std;

int main() {
    const string SRC {"Hello, world!"};
    cout << SRC << endl;

    string dst;
    transform(
        SRC.begin(),
        SRC.end(),
        back_inserter(dst),
        [](unsigned char ch) { return toupper(ch); }
    );
    cout << dst << endl;

    return 0;
}
```

```
Hello, world!
HELLO, WORLD!
```
