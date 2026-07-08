#include <iostream>
#include <format>

int main() {
    int count{42}; // <-- breakpoint здесь
    unsigned char byte_val{0xA5}; // 165 в десятичном, удобно для /x /t /o
    char letter = 'Z';
    int numbers[5] = {10, 20, 30, 40, 50};
    int* ptr = &numbers[2];

    for (int i{}; i < 5; ++i) {
        count += numbers[i]; // <-- сюда попробуем display count / display/x count
        std::cout << std::format("count: {}\n", count);
    }

    std::cout << std::format("byte_val: {}\n", static_cast<int>(byte_val));
    std::cout << std::format("letter: {}\n", letter);
    std::cout << std::format("*ptr: {}\n", *ptr);

    return 0;
}

/*

###
lldb .\build\debug\app.exe
breakpoint set --file main.cpp --line 12
run
target stop-hook add -o 'frame variable count'
print/x byte_val
expression -f binary -- byte_val
continue

###
C:\msys64\clang64\bin\gdb.exe .\build\debug\app.exe
break main.cpp:12
run
display count
display/x count
print byte_val
print/x byte_val
print/t byte_val
continue

*/
