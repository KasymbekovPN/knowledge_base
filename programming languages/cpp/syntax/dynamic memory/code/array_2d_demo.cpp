#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    size_t rows {3};
    size_t columns {2};
    int** numbers {new int*[rows]{}};

    for (size_t i{}; i < rows; i++) {
        numbers[i] = new int[columns]{};
    }

    for (size_t i {}; i < rows; i++) {
        for (size_t j {}; j < columns; j++) {
            cout << numbers[i][j] << " ";
        }
        cout << "\n";
    }

    for (size_t i {}; i < rows; i++) {
        delete[] numbers[i];
    }
    delete[] numbers;
    
    return 0;
}
