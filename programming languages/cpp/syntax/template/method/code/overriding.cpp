#include <iostream>

template<typename T> const T* max(const T*, const T*);
template<typename T> const T* max(const T[], unsigned);

int main(int argc, char const *argv[]) {
    const int A{42};
    const int B{45};
    std::cout << "first variant:\t" << *max(&A, &B) << std::endl;

    const double NUMBERS[] {1.2, 2.3, 3.4, 4.5};
    std::cout << "second variant:\t" << *max(NUMBERS, std::size(NUMBERS)) << std::endl;

    return 0;
}

template<typename T> const T* max(const T* a, const T* b) {
    return *a > *b ? a : b;
}

template<typename T> const T* max(const T data[], unsigned size) {
    const T* result {};
    for (size_t i{}; i < size; i++) {
        if (!result || data[i] > *result) {
            result = &data[i];
        }
    }
    
    return result;
}
