#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int first {1};
    int second {42};

    int* poiner_first {&first};
    int* pointer_second {&second};

    cout << "*pointer_first <= " << *poiner_first << endl;
    cout << "*pointer_second <= " << *pointer_second << endl;

    pointer_second = poiner_first;
    cout << "*pointer_first <= " << *poiner_first << endl;
    cout << "*pointer_second <= " << *pointer_second << endl;

    return 0;
}
