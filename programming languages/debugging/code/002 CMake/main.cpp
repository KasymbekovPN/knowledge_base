#include <iostream>
#include <format>

int main() {
    constexpr int SIZE{3};
    for (int i{}; i < SIZE; ++i) {
        int sq{i * i};
        std::cout << std::format("sq: {}\n", sq);
    }

    return 0;
}

/*

cmake --preset release
cmake --build --preset release
cmake --build --preset release --config=Release

###

cmake --preset debug
cmake --build --preset debug
cmake --build --preset debug --config=Debug

###

cmake --preset relwithdebinfo
cmake --build --preset relwithdebinfo
cmake --build --preset relwithdebinfo --config=RelWithDebInfo

###

lldb ./build/debug/app.exe
lldb ./build/relwithdebinfo/app.exe
breakpoint set --file main.cpp --line 8
run
frame variable
bt
continue
continue
print i
print sq



C:\msys64\clang64\bin\gdb.exe ./build/debug/app.exe
C:\msys64\clang64\bin\gdb.exe ./build/relwithdebinfo/app.exe
break main.cpp:8
run
info locals
bt
continue
continue
print i
print sq

 */