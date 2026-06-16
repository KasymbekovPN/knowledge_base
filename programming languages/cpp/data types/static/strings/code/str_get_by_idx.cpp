#include <iostream>
#include <string>

using std::cout;
using std::endl;
using std::string;

int main(int argc, char const *argv[]) {
    string hello {"Hello !!!"};
    cout << "Original hello <= '"  << hello << "'" << endl;

    hello[0] = 'h';
    cout << "Changed hello <= '"  << hello << "'" << endl;

    char first_char = {hello[0]};
    cout << "First char <= '" << first_char << "'" << endl;

    const char FIND_CHAR = 'l';
    unsigned count {};
    for (const char ch : hello) {
        count += ch == FIND_CHAR ? 1 : 0;
    }
    cout << "'l' counter <= " << count << endl;

    return 0;
}
