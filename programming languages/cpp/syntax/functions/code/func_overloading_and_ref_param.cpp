#include <iostream>

using std::cout;
using std::endl;

int print(int);
int print(int&);
int print(double);

int main(int argc, char const *argv[]) {
    const int number {42};

    print(number);
    print(&number);
    print((double) number);

    return 0;
}

int print(int number) {
    cout << "[print(int)] " << number << endl;
}

int print(int& number) {
    cout << "[print(int&)] " << number << endl;
}

int print(double number) {
    cout << "[print(double)] " << number << endl;
}
