#include <iostream>

using std::cout;
using std::endl;

class Demo {
public:
    Demo() {cout << "ctor" <<endl;}
    Demo(const Demo&) {cout << "copy ctor" << endl;}
    Demo(const Demo&&) noexcept {cout << "move ctor" << endl;}
};

Demo create_rvo() {
    return Demo();
}

Demo create_nvro() {
    Demo obj;
    return obj;
}

Demo create_move() {
    Demo obj;
    return std::move(obj);
}

int main() {
    cout << "Call create_rvo" << endl;
    create_rvo();

    cout << "Call create_nrvo" << endl;
    create_nvro();

    cout << "Call create_move" << endl;
    create_move();

    return 0;
}
