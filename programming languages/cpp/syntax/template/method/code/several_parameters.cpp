#include <iostream>

template <typename T, typename K> void print(T, T, K, unsigned);

int main(int argc, char const *argv[]) {
    ::print("hello", "world", -123, 123);
    ::print(12.3, 34.5, std::string{"hello"}, 999);

    return 0;
}

template <typename T, typename K>
void print(T t0, T t1, K k, unsigned u) {
    std::cout
        << "{t0: " << t0
        << ", t1: " << t1
        << ", k: " << k
        << ", u: " << u
        << "}" << std::endl;
}
