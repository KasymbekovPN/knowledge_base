#include <iostream>

int main(int argc, char const *argv[]) {
    int number {5};
    const int &ref {number};

    ref = 111;

    return 0;
}
