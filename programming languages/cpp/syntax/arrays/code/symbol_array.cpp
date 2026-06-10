#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    const char hello0[] {'h', 'e', 'l', 'l', 'o'};
    const char hello1[] {'h', 'e', 'l', 'l', 'o', '\0'};
    const char hello2[] {"hello"};

    cout << "hello0 <= " << hello0 << endl;
    cout << "hello1 <= " << hello1 << endl;
    cout << "hello2 <= " << hello2 << endl;

    return 0;
}
