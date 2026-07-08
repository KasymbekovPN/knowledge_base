#include <iostream>

int main(int argc, char *argv[]) {
    for (int i{7}; i < 10; ++i) {
        std::cout << i << std::endl;
        std::cout << i*i << std::endl;
        std::cout << i*i*i << std::endl;
    }

    return 0;
}

/*

###
lldb .\build\debug\app.exe
breakpoint set --file main.cpp --line 5
breakpoint set --file main.cpp --line 6
breakpoint set --file main.cpp --line 7
breakpoint list
breakpoint disable 1
breakpoint disable 3

###
C:\msys64\clang64\bin\gdb.exe .\build\debug\app.exe
break main.cpp:5
break main.cpp:6
break main.cpp:7
info breakpoints
disable 1
disable 3

*/
