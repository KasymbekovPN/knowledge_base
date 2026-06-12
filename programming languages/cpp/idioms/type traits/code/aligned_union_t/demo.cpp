#define _ENABLE_EXTENDED_ALIGNED_STORAGE
// #define _DISABLE_EXTENDED_ALIGNED_STORAGE

#include <iostream>
#include <type_traits>
#include <new>

struct Small {
    int x;
};

struct Big {
    double data[10];
};

struct Aligned {
    alignas(16) char buffer[48];
};

using Storage = std::aligned_union_t<2, Small, Big, Aligned>;

int main() {
    alignas(Storage) char raw_memory[sizeof(Storage)];
    void* mem = raw_memory;

    Big* b = new(mem) Big{};
    b->data[0] = 3.14;

    std::cout << "Value: " << b->data[0] << std::endl;

    b->~Big();

    return 0;
}
