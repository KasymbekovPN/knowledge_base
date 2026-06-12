#include <iostream>

using std::cout;
using std::endl;

void print(std::string, unsigned);

int main(int argc, char const *argv[]) {
    print("Sam", 30);

    return 0;
}

void print(std::string name, unsigned age) {
    cout << "{name: " << name << ", age: " << age << "}";
}
