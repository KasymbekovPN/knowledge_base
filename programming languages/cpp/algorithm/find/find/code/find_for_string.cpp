#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

void _test_find(const string&, char);

int main() {
    const string str {"Hello, world !!!"};
    const vector<char> chs {'x', 'o', 'z'};
    for (auto& ch: chs) {
        _test_find(str, ch);
    }

    return 0;
}

void _test_find(const string& str, char ch) {
    auto it = std::find(str.begin(), str.end(), ch);
    if (it != str.end()) {
        cout
            << "Found '" << ch << "' at position "
            << (it - str.begin()) << endl;
    } else {
        cout << "'" << ch << "' not found" << endl;
    }
}
