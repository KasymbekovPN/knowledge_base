#include <iostream>
#include <format>
#include <string>

int level_three(const int x) {
    const int squared{x * x};
    const int doubled{2 * x};

    return squared + doubled; // <-- breakpoint здесь, самый глубокий фрейм
}

int level_two(const int a, const int b) {
    std::string label{"processing level_two"};
    const int sum{a + b};
    const int result{level_three(sum)};

    return 2 * result;
}

int level_one(const int input) {
    std::string context{"top-level call"};
    const int adjusted{10 + input};
    const int final_result{level_two(adjusted, input)};

    return final_result;
}

int main() {
    constexpr int value{5};
    const int answer{level_one(value)};

    std::cout << std::format("answer: {}\n", answer);

    return 0;
}

/*

###
lldb .\build\debug\app.exe
breakpoint set --file main.cpp --line 9
run
register read
register read rax rbx
register read --all
print $rip
print $rsp

###
C:\msys64\clang64\bin\gdb.exe .\build\debug\app.exe
break main.cpp:9
run
info registers
info registers rax
print $rip
print $rsp
print $rbp
print/x $rax
info all-registers

*/
