#include <iostream>
#include <forward_list>

int main() {
    std::forward_list<int> flist {1, 2, 3};
    std::cout
        << "First element: "
        << flist.front()
        << std::endl;

    return 0;
}
