#include <iostream>

double divide(double, double, int);
void test(int);

int main(int argc, char const *argv[]) {
    const int KINDS[] {0, 1};
    for (auto kind: KINDS) {
        ::test(kind);
    }

    return 0;
}

double divide(double a, double b, int kind) {
    if (!b) {
        if (kind == 0) {
            throw std::exception("Divide by zero");
        } else {
            throw std::string("Divide by zero");
        }
    }
    return a / b;
}

void test(int kind) {
    try {
        try {
            ::divide(100.0, 0.0, kind);
        } catch(const std::string& e) {
            std::cerr << "std::string <= " << e << std::endl;
        }
        
    } catch(const std::exception& e) {
        std::cerr << "std::exception <= " << e.what() << std::endl;
    } catch(...) {
        std::cerr << "Unknown exception" << std::endl;
    }
}
