#include <iostream>
#include <format>
#include <cstdlib>

// Маленькая функция — идеальный кандидат на инлайнинг компилятором
inline int square(int x) {
    int result{x * x}; // <-- сюда попробуем step внутрь
    return result;
}

int compute(int a, int b) {
    int sq_a = square(a); // <-- breakpoint здесь: step vs next
    int sq_b = square(b);
    int sum = sq_a + sq_b; // <-- смотрим, куда попали после finish/step
    return sum;
}

int main(int argc, char *argv[]) {
    int x = argc > 1 ? std::atoi(argv[1]) : 3;
    int y = argc > 2 ? std::atoi(argv[2]) : 4;
    std::cout << std::format("result = {}\n", compute(x, y));

    return 0;
}

/*

# GDB
C:\msys64\clang64\bin\gdb.exe --args .\build\debug\app.exe 6 7
C:\msys64\clang64\bin\gdb.exe --args .\build\release\app.exe 6 7
break main.cpp:12
run
bt
step

# LLDB
lldb -- .\build\debug\app.exe 6 7
lldb -- .\build\release\app.exe 6 7
breakpoint set --file main.cpp --line 12
run
bt
step

*/