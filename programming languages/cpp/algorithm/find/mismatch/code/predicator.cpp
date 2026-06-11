#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>

using namespace std;

bool _case_insensitive_eq(char, char);
void _test_mismatch(const string&, const string&, bool(*)(char, char));

int main() {
    const string s0 = "Hello";
    const string s1 = "hELLo";
    const string s2 = "hE11o";

    _test_mismatch(s0, s1, _case_insensitive_eq);
    _test_mismatch(s1, s2, _case_insensitive_eq);

    return 0;
}

bool _case_insensitive_eq(char a, char b) {
    return 
        tolower(static_cast<unsigned char>(a)) ==
        tolower(static_cast<unsigned char>(b));
}

void _test_mismatch(const string& str0, const string& str1, bool(*pred)(char, char)) {
    auto [it0, it1] = mismatch(str0.begin(), str0.end(), str1.begin(), pred);
    if (it0 != str0.end()) {
        cout << "Mismatch at position " << distance(str0.begin(), it0) << endl;
    } else {
        cout << "Strings are equal (case-insensitive)" << endl;
    }
}
