#include <iostream>

void func0(const int (*)[3], const size_t);
void func1(const int [][3], const size_t);

int main(int argc, char const *argv[]) {
    int table [][3] {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    size_t rawsCount {std::size(table)};

    func0(table, rawsCount);
    func1(table, rawsCount);

    return 0;
}

void func0(const int (*rows)[3], const size_t rowsCount) {
    std::cout << "func0" << std::endl;
    size_t columnsCount {std::size(*rows)};
    for (size_t i {}; i < rowsCount; i++) {
        for (size_t j {}; j < columnsCount; j++){
            std::cout << rows[i][j] << "\t";
        }
        std::cout << std::endl;
    }
}

void func1(const int rows [][3], const size_t rowsCount) {
    std::cout << "func1" << std::endl;
    size_t columnsCount {std::size(rows[0])};
    for (size_t i {}; i < rowsCount; i++) {
        for (size_t j {}; j < columnsCount; j++){
            std::cout << rows[i][j] << "\t";
        }
        std::cout << std::endl;
    }
}
