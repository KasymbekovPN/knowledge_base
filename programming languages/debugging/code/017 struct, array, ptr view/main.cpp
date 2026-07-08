#include <iostream>
#include <string>

struct Point {
    int x;
    int y;
};

struct Employee {
    std::string name;
    int age;
    Point office;      // вложенная структура
    double salary[3];  // массив внутри структуры (бонусы по кварталам)
};

int main() {
    int count = 42;                 // <-- breakpoint здесь
    unsigned char byte_val = 0xA5;   // 165 в десятичном, удобно для /x /t /o
    char letter = 'Z';
    int numbers[5] = {10, 20, 30, 40, 50};
    int* ptr = &numbers[2];

    Point p1{10, 20};
    Employee emp{"Pablo", 35, Point{1, 4}, {1000.0, 1500.0, 1200.0}};
    Employee* emp_ptr = &emp;

    for (int i = 0; i < 5; ++i) {
        count += numbers[i];        // <-- сюда попробуем display count / display/x count
        std::cout << "count = " << count << "\n";
    }

    std::cout << "byte_val = " << static_cast<int>(byte_val) << "\n";
    std::cout << "letter = " << letter << "\n";
    std::cout << "*ptr = " << *ptr << "\n";
    std::cout << "emp.name = " << emp.name << ", office=(" << emp.office.x << "," << emp.office.y << ")\n";  // <-- breakpoint сюда
    return 0;
}

/*

###
lldb .\build\debug\app.exe
breakpoint set --file main.cpp --line 29
run
print emp.age
print emp.office.x
print emp_ptr->name
print numbers
print numbers[2]
print ptr
print *ptr

###
C:\msys64\clang64\bin\gdb.exe .\build\debug\app.exe
break main.cpp:29
run
print emp.age
print emp.office.x
print emp_ptr->name
print numbers
print numbers[2]
print ptr
print *ptr

*/
