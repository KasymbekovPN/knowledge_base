#include <iostream>

[[nodiscard("int-compute")]]
static int compute() { return 42; }

namespace {
    struct [[nodiscard("ErrorCode")]] ErrorCode { int value; };
}

static ErrorCode get_error_code_v0() { return ErrorCode(); }

[[nodiscard("get_error_code_v1")]]
static ErrorCode get_error_code_v1() { return ErrorCode(); }

int main() {
    compute();
    get_error_code_v0();
    get_error_code_v1();
    std::cout << compute() << std::endl;

    return 0;
}
