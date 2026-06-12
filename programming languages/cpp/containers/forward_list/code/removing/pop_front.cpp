#include <iostream>
#include <forward_list>

template<typename T>
void print_flist(const std::forward_list<T>&);

template<typename T>
void _pop_front(std::forward_list<T>&);

int main() {
    std::forward_list<int> flist {1, 2, 3};
    print_flist(flist);

    for (size_t i{}; i < 4; i++) {
        _pop_front(flist);
        print_flist(flist);
    }

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

template<typename T>
void _pop_front(std::forward_list<T>& flist) {
    if (!flist.empty()) {
        flist.pop_front();
    }
}
