#include <iostream>

void test(bool);
int divide(int, int);
double divide(double, double);

int main(int argc, char const *argv[]) {
    const bool arr[] {false, true};
    for (auto f: arr) {
        ::test(f);
    }

    return 0;
}

void test(bool f) {
    try {
        if (f) {
            divide(100, 0);
        } else {
            divide(100.0, 0.0);
        }
    }
    catch(const char* error_message) {
        std::cerr << error_message << std::endl;
    } catch (...) {
        std::cerr << "EXCEPTION" << std::endl;
    }
}

int divide(int a, int b) {
    if (b) {
        return a / b;
    }
    throw "Division by zero";
}

double divide(double a, double b) {
    if (b) {
        return a / b;
    }

    throw -1;
}
