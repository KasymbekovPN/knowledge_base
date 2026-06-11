#include <iostream>
#include <cstring>

template <size_t N>
size_t get_array_size(const char (&)[N]) {
    return N;
}

int main() {
    const char line[] = {'!', '!', '!'};
    std::cout << "len <= " << get_array_size(line) << std::endl;

    return 0;
}
