#include <iostream>
#include <algorithm>

using namespace std;

int main() {
    string line {"Hello, world !!!"};
    cout << line << endl;

    reverse(line.begin(), line.end());
    cout << line << endl;

    return 0;
}
