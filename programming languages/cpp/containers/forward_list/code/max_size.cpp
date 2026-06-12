#include <iostream>
#include <forward_list>

int main() {
    std::forward_list<int> flist;
    std::cout << "max_size => " << flist.max_size() << std::endl;

    return 0;
}
