#include <iostream>
#include <forward_list>

struct CompareDescending {
    bool operator()(int a, int b) {
        return a > b;
    }
};

template<typename T>
void print_flist(const std::forward_list<T>&);

int main() {
    std::forward_list<int> numbers {123, 17, 200, 1, -1};
    print_flist(numbers);

    numbers.sort(CompareDescending());
    print_flist(numbers);

    return 0;
}

template<typename T>
void print_flist(const std::forward_list<T>& flist) {
    std::cout << "flist => ";
    for (const auto &item: flist) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}
