#include <iostream>
#include <format>

template <typename T>
struct Value {
    T value;

    explicit Value(T _value) noexcept : value{_value} {}
    T getValue() const noexcept { return value; }
};

void test_print() {
    std::cout << "test_print\n";
}

int main(int argc, char *argv[]) {
    constexpr int value{123};
    std::cout << std::format("value: {}\n", value);

    test_print();

    const Value<double> v{12.34};
    std::cout << std::format("v.getValue(): {}\n", v.getValue());

    return 0;
}

/*

lldb .\build\debug\app.exe
breakpoint set --name main
b test_print
breakpoint set --name "Value<double>::getValue"
run

C:\msys64\clang64\bin\gdb.exe .\build\debug\app.exe
break main
break test_print
break Value<double>::getValue
run

 */