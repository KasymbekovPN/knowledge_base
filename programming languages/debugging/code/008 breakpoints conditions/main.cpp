#include <iostream>
#include <format>

int main(int argc, char *argv[]) {

    for (int i = 1; i < 10; i++) {
        std::cout << std::format("i: {}\n", i);
    }

    for (int i = 1; i < 10; i++) {
        std::cout << std::format("i: {}\n", i);
    }

    return 0;
}

/*

lldb .\build\debug\app.exe
breakpoint set --file main.cpp --line 7 --condition "i == 5"
breakpoint set --file main.cpp --line 11 --condition "i == 7"
run

C:\msys64\clang64\bin\gdb.exe .\build\debug\app.exe
break main.cpp:7 if i == 5
break main.cpp:11 if i == 7
run

 */