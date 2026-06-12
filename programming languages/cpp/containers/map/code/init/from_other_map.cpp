#include <iostream>
#include <map>

using std::cout;
using std::endl;
using std::map;
using std::string;

void _print_map(const map<string, int>&);

int main() {
    const map<string, int> original_map {
        {"hello", 100},
        {"world", 500},
    };
    const map<string, int> map {original_map};

    _print_map(map);

    return 0;
}


void _print_map(const map<string, int>& m) {
    for (auto &pair: m) {
        cout << "{" << pair.first
            << ", " << pair.second
            << "}" << endl;
    }
}
