#include <iostream>

enum class Operation {
    add,
    sub,
    mul
};

void execute(int, int, Operation);

int main(int argc, char const *argv[]) {
    execute(10, 15, Operation::add);
    execute(10, 15, Operation::sub);
    execute(10, 15, Operation::mul);

    return 0;
}

void execute(int i0, int i1, Operation op) {
    switch (op) {
        case Operation::add:
            std::cout << i0 << " + " << i1 << " <= " << i0 + i1 << std::endl;
            break;
        case Operation::sub:
            std::cout << i0 << " - " << i1 << " <= " << i0 - i1 << std::endl;
            break;
        case Operation::mul:
            std::cout << i0 << " * " << i1 << " <= " << i0 * i1 << std::endl;
            break;
    }
}
