#include "include/rust_math_lib.h"
#include <iostream>

int main() {
    std::cout << "rust_add(2, 3) = " << rust_add(2, 3) << "\n";

    Vector3 a{1.0, 2.0, 2.0};
    std::cout << "vector3_length = " << vector3_length(a) << "\n";

    Accumulator* acc = accumulator_new();
    accumulator_add(acc, 10.5);
    accumulator_add(acc, 5.5);
    std::cout << "total = " << accumulator_total(acc) << "\n";
    accumulator_free(acc); // обязательно -- иначе утечка на Rust-стороне

    accumulator_add(nullptr, 100.0); // проверено Rust-стороной, не падает
    return 0;
}