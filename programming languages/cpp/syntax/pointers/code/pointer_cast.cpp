#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    char ch {'x'};
    char* pch {&ch};
    int* pnumber {(int*)pch};
    void* pvoid {(void*)pch};

    cout << "pch: " << pch << endl;
    cout << "pnumber: " << pnumber << " | *pnumber: " << *pnumber << endl;
    cout << "pvoid: "   << pvoid   << endl;

    return 0;
}
