#include <iostream>

using std::cout;
using std::endl;

void func(const int, const std::string);

int main(int argc, char const *argv[]) {
    func(0, "first");
    func(1, "second");
    return 0;
}

void func(const int value, const std::string key) {
    cout << "[" << key << "] value <= " << value << endl;
    if (value <= 0) {
        return;
    }

    cout << "[" << key << "] it's positive " << endl;
}
