#include <iostream>

template <typename T> decltype(auto) average(T* p_begin, T* p_end) {
    size_t size {};
    T result{};
    for (T* it{p_begin}; it != p_end; it++) {
        result += *it;
        size++;
    }

    return result / size;
}

int main(int argc, char const *argv[]) {
    int NUMBERS[] {1, 23, 14, 42, 89};
    std::cout << "average <= " << average(std::begin(NUMBERS), std::end(NUMBERS)) << std::endl;
    return 0;
}
