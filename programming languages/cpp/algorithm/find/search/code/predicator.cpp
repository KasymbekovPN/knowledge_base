#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>

using namespace std;

bool _case_insensitive_eq(char, char);
void _test_search(string&, string&, bool(*)(char, char));

int main() {
    string text {"Hello world! HELLO universe!"};
    string pattern0 {"HELLO"};
    string pattern1 {"abc"};

    _test_search(text, pattern0, _case_insensitive_eq);
    _test_search(text, pattern1, _case_insensitive_eq);

    return 0;
}

bool _case_insensitive_eq(char a, char b) {
    return
        tolower(static_cast<unsigned char>(a)) ==
        tolower(static_cast<unsigned char>(b));
}

void _test_search(string& text, string& pattern, bool(*pred)(char, char)) {
    auto it = search(
        text.begin(),
        text.end(),
        pattern.begin(),
        pattern.end(),
        pred
    );

    if (it != text.end()) {
        cout 
            << "Pattern found at position: "
            << distance(text.begin(), text.begin())
            << endl
            << "Matched substring: " << string(it, it + pattern.size())
            << endl;
    } else {
        cout << "Not found" << endl;
    }
}
