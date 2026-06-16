#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    size_t n{3};
    int (*ptr)[2] = new int[n][2];
    int k{};

    for (size_t i {}; i < n; i++) {
        for (size_t j {}; j < 2; j++) {
            ptr[i][j] = ++k;
        }
    }
    
    for (size_t i {}; i < n; i++) {
        for (size_t j {}; j < 2; j++) {
            cout << ptr[i][j] << " ";
        }
        cout << endl;
    }

    delete[] ptr;
    ptr = nullptr;

    return 0;
}
