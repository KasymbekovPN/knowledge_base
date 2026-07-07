#include <iostream>
#include <format>

int main(int argc, char *argv[]) {

    int a{};
    a = 42;
    std::cout << a << std::endl;

    return 0;
}

/*

lldb .\build\debug\app.exe
breakpoint set --name main
run
watchpoint set variable a --watch read_write
continue

C:\msys64\clang64\bin\gdb.exe .\build\debug\app.exe
break main
run
awatch a
continue

 */
