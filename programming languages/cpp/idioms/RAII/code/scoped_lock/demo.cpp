#include <iostream>
#include <mutex>

using namespace std;

int main() {
    mutex mtx_a;
    int value_a {13};

    mutex mtx_b;
    int value_b {42};

    scoped_lock lock (mtx_a, mtx_b);
    cout << value_a << ", " << value_b << endl;

    return 0;
}
