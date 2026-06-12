#include <iostream>
#include <array>

template<typename T, int N>
void print_array(const std::string& label, const std::array<T,N>& array) {
    std::cout << label << " : ";
    for (auto &&item: array) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}

int main() {
    std::array<int, 3> numbers0 {1, 2, 3};
    std::array<int, 3> numbers1 {4, 5, 6};

    print_array<int, 3>("numbers0", numbers0);
    print_array<int, 3>("numbers1", numbers1);

    numbers0.swap(numbers1);
    print_array<int, 3>("numbers0", numbers0);
    print_array<int, 3>("numbers1", numbers1);

    return 0;
}
