---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/find/find_end/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <cctype>

using namespace std;

void _test_find_end(const string&, const string&, bool(*)(char, char));
bool _case_insensitive_eq(char, char);

int main() {
    string text = "This is a test string. This is another.";
    vector<string> patterns = {
        "THIS",
        "is",
        "abc"
    };

    for(auto& pattern: patterns) {
        _test_find_end(text, pattern, _case_insensitive_eq);
    }

    return 0;
}

void _test_find_end(const string& text,
					const string& pattern,
					bool(*pred)(char, char)) {
    auto it = find_end(
        text.begin(),
        text.end(),
        pattern.begin(),
        pattern.end(),
        pred
    );

    if (it != text.end()) {
        cout
            << "Pattern ends at position: "
            << distance(text.begin(), it)
            << endl;
    } else {
        cout << "Pattern not found" << endl;
    }
}

bool _case_insensitive_eq(char a, char b) {
    return
        tolower(static_cast<unsigned char>(a)) ==
        tolower(static_cast<unsigned char>(b));
}
```

```
Pattern ends at position: 23
Pattern ends at position: 28
Pattern not found
```
