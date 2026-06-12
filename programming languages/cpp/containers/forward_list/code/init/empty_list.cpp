#include <iostream>
#include <forward_list>

int main() {
    std::forward_list<int> flist;
    std::cout << "Is empty? " << (flist.empty() ? "Yes" : "No") << std::endl;

    return 0;
}
