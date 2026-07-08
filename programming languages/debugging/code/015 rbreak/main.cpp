#include <iostream>
#include <format>

template<typename T>
struct Value {
    T value;

    explicit Value(T _value) noexcept : value(_value) {}

    T get() const noexcept {
        return value;
    }
};

void test_int(const int x) {
    std::cout << std::format("test_int: {}\n", x);
}

void test_double(const double x) {
    std::cout << std::format("test_double: {}\n", x);
}

void print_str(const std::string s) {
    std::cout << std::format("print_str: {}\n", s);
}

int main(int argc, char *argv[]) {
    test_int(1234);
    test_double(12.34);
    print_str("hello world");

    const Value<int> v{42};
    std::cout << v.get() << '\n';

    return 0;
}

/*

###
lldb .\build\debug\app.exe
breakpoint set --func-regex "test|print"
breakpoint set --func-regex "Value<.*>::.*"

###
C:\msys64\clang64\bin\gdb.exe .\build\debug\app.exe
rbreak test\|print
rbreak ^Value::<.*>

*/
