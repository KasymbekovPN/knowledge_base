#include <iostream>

int main(int argc, char *argv[]) {
    for (int i{7}; i < 10; ++i) {
        std::cout << i << std::endl;
    }

    return 0;
}

/*

###
lldb .\build\debug\app.exe
breakpoint set --file main.cpp --line 5 --one-shot true

###
C:\msys64\clang64\bin\gdb.exe .\build\debug\app.exe
tbreak main.cpp:5

*/