#include <iostream>
#include <memory>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    std::unique_ptr<int> no_init_ptr {};
    cout << "no_init_ptr <= " << no_init_ptr << endl;

    std::unique_ptr<int> ptr {std::make_unique<int>(42)};
    cout << "Address <= " << ptr.get() << endl;
    cout << "Value <= " << *ptr << endl;

    *ptr = 101;
    cout << "Value <= " << *ptr << endl;

    ptr.reset(new int{-111});
    cout << "Value <= " << *ptr << endl;

    ptr.reset();
    if (!ptr) {
        cout << "ptr is free" << endl;
    }

    cout << "Done" << endl;

    return 0;
}
