#include <iostream>
#include <utility>

using std::cout;
using std::endl;

int main() {
    std::string str {"Hello"};
    std::string mstr = std::move(str);

    // str is valud but undefined
    // possible empty
    cout << "str:  '" << str << "'" << endl;
    // mstr took inner data str
    cout << "mstr: '" << mstr << "'" << endl;

    return 0;
}
