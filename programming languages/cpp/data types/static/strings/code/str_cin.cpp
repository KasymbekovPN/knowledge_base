#include <iostream>
#include <string>

using std::cout;
using std::cin;
using std::endl;
using std::string;

int main(int argc, char const *argv[]) {
    string line0;
    cout << "Input line0: ";
    getline(cin, line0);
    cout << "line0 <= '" << line0 << "'" << endl;

    string line1;
    cout << "Input line1: ";
    cin >> line1;
    cout << "line1 <= '" << line1 << "'" << endl;

    return 0;
}
