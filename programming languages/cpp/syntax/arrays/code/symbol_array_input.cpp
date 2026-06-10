#include <iostream>

using std::cout;
using std::cin;
using std::endl;

int main(int argc, char const *argv[]) {
    const int MAX_LEN {64};
    char text[MAX_LEN] {};

    cout << "Enter text:" << endl;
    cin.getline(text, MAX_LEN);

    cout << "Input: " << text << endl;

    return 0;
}
