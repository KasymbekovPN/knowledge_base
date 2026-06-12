#include <iostream>

using BinaryOp = int (*)(int, int);

int sum(int, int);
int sub(int, int);
int do_operation(BinaryOp, int, int);

int main(int argc, char const *argv[]) {
    int first {6};
    int second {9};

    std::cout << "Sum of " << first << " & " << second << " => " << do_operation(sum, first, second) << std::endl;
    std::cout << "Sub of " << first << " & " << second << " => " << do_operation(sub, first, second) << std::endl;

    return 0;
}

int sum(int first, int second) {
    return first + second;
}

int sub(int first, int second) {
    return first - second;
}

int do_operation(BinaryOp op, int first, int second) {
    return op(first, second);
}
