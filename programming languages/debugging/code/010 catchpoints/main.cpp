#include <iostream>
#include <format>
#include <stdexcept>
#include <string>

void validate(int value) {
    if (value >= 0) return;
    throw std::invalid_argument(std::format("value must be non-negative: {}", value));
}

int risky_divide(int a, int b) {
    if (b == 0) {
        throw std::runtime_error("division by zero");
    }
    return a / b;
}

int main() {
    try {
        validate(5);
        validate(-6);
    } catch (const std::invalid_argument& e) {
        std::cout << std::format("Caught invalid_argument: {}\n", e.what());
    }

    try {
        risky_divide(5, 0);
    } catch (const std::runtime_error& e)  {
        std::cout << std::format("Caught runtime_error: {}\n", e.what());
    }

    std::cout << "Done\n";
    return 0;
}

/*

###
lldb .\build\debug\app.exe
breakpoint set --name _CxxThrowException
run
bt
continue
bt

###
C:\msys64\clang64\bin\gdb.exe .\build\debug\app.exe
set breakpoint pending on
break _CxxThrowException
run
bt
continue
bt

*/