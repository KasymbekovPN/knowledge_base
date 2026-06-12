#include <iostream>

int gl_var;

int main(){
    int age0 = 20;
    std::cout << "Age 0: " << age0 << "\n";
    int age1{21};
    std::cout << "Age 1: " << age1 << "\n";
    int age2(22);
    std::cout << "Age 2: " << age2 << "\n";

    int age10 = 20.1;
    std::cout << "Age 10: " << age10 << "\n";
    int age12(22.1);
    std::cout << "Age 12: " << age12 << "\n";
    // int age11{21.1};
    // std::cout << "Age 11: " << age11 << "\n";

    int age21 {};
    std::cout << "Age 21 = " << age21 << "\n";

	int scope_var;
	std::cout << "gl_val: " << gl_var << "\n";
	std::cout << "scope_val: " << scope_var << "\n";

    return 0;
}
