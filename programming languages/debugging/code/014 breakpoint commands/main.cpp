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
breakpoint set --file main.cpp --line 5
breakpoint command add 1
> print i
> continue
> DONE

###
C:\msys64\clang64\bin\gdb.exe .\build\debug\app.exe
break main.cpp:5
commands 1
> print i
> continue
> end

*/
