#include <iostream>
#include <string>

using std::string;
using std::cout;
using std::endl;
using std::begin;
using std::end;

void print(string, int*, int*);

int main(int argc, char const *argv[]) {
    const int SIZE {4};
    int* numbers0 {new int[SIZE]{}};
    int* numbers1 {new int[SIZE]{1, 2, 3, 4}};
    int* numbers2 {new int[SIZE]{1, 2}};
    int* numbers3 {new int[]{1, 2}};

    print("numbers0", numbers0, numbers0 + SIZE);
    print("numbers1", numbers1, numbers1 + SIZE);
    print("numbers2", numbers2, numbers2 + SIZE);
    print("numbers3", numbers3, numbers3 + 2);

    return 0;
}

void print(string label, int* begin, int* end) {
    cout << "[print] " << label << " {";
    for (int* it {begin}; it != end; it++) {
        cout << " " << *it;
    }
    cout << "}" << endl;
}
