#include <iostream>
#include <forward_list>

template<typename T>
void print_flist(const std::forward_list<T>&);

int main() {
    std::forward_list<int> flist {1, 2, 3, 4, 5};
    print_flist(flist);

    return 0;
}

template<typename T>
void print_flist(const std::forward_list<T>& flist) {
    for (const auto &item: flist) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}
