#include <iostream>

using std::cout;
using std::endl;

int sum(int, int);
int sub(int, int);
int mul(int, int);

int (*select(int))(int, int);

int main(int argc, char const *argv[]) {
    int first {43};
    int second {13};

    int (*action)(int, int) {select(0)};
    cout << "Sum of " << first << " & " << second << " => " << action(first, second) << endl;

    cout << "Sub of " << first << " & " << second << " => " << select(1)(first, second) << endl;

    cout << "Mul of " << first << " & " << second << " => " << select(2)(first, second) << endl;

    return 0;
}

int sum(int first, int second) {
    return first + second;
}

int sub(int first, int second) {
    return first - second;
}

int mul(int first, int second) {
    return first * second;
}

int (*select(int choice))(int, int) {
    switch (choice)
    {
        case 1:
            return sub;
        case 2:
            return mul;
    }

    return sum;
}
