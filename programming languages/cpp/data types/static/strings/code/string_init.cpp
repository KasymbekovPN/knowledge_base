#include <iostream>
#include <string>

using std::cout;
using std::endl;
using std::string;

int main(int argc, char const *argv[]) {
    string empty_msg;
    string msg0 {"some message !!!"};
    string msg1 {msg0};
    string msg2 (msg0);
    string msg3 = msg0;

    cout << "empty_msg <= '" << empty_msg << "'" << endl;
    cout << "msg0 <= '" << msg0 << "'" << endl;
    cout << "msg1 <= '" << msg1 << "'" << endl;
    cout << "msg2 <= '" << msg2 << "'" << endl;
    cout << "msg3 <= '" << msg3 << "'" << endl;

    return 0;
}
