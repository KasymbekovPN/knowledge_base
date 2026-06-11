#include <iostream>
#include <algorithm>
#include <string>

using namespace std;

int main() {
    const string SOURCE {"hello, world !!!"};

    char dst[20] = {};
    copy(SOURCE.begin(), SOURCE.end(), dst);
    dst[SOURCE.size()] = '\0';

    cout << "dst: " << dst << endl;

    return 0;
}
