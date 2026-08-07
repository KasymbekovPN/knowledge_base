#include "model.generated.h"

#include <iostream>

int main() {
    Person p(30, "Ada Lovelace", 123456.78);
    std::cout << to_json(p) << "\n";

    return 0;
}

/*

& "C:\Users\Pavel Kasymbekov\AppData\Local\Programs\Python\Python311\python.exe" -m venv .venv
pip install libclang --break-system-packages
.venv\Scripts\Activate.ps1
python.exe .\generate.py
clang++.exe -std=c++17 main.cpp

*/