#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    char langs[][20] = {"C++", "Python", "JavaScript"};
    cout << "Size of langs[0] <= " << std::size(langs[0]) << " bytes" << endl;

    const char* plangs[] {"C++", "Python", "JavaScript"};
    for (size_t i{}; i < std::size(plangs); i++) {
        cout << "'" << plangs[i] << "'" << endl;
    }
    
    return 0;
}
