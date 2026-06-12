#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int a {8};
    int b {7};
    int c {a + 5 * ++b};

    int count {1};
    int result {++count * 3 + count++ * 5};

    int d {8};
    int e {7};
    int f {(a + 5) * ++b};

    cout << "a <= " << a << endl;
    cout << "b <= " << b << endl;
    cout << "c = a + 5 ++b <= " << c << endl;
    cout << endl;
    cout << "count <= " << count << endl;
    cout << "result = ++count * 3 + count++ * 5 <= " << result << endl;
    cout << endl;
    cout << "d <= " << d << endl;
    cout << "e <= " << e << endl;
    cout << "f = (a + 5) * ++b <= " << f << endl;

    return 0;
}
