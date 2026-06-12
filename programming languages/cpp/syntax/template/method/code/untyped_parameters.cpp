#include <iostream>

template <typename T, size_t N=1> void print(const T&);
template <typename T, size_t N> T get(const T(&)[N], const size_t);
template <typename T, size_t N> T average(const T(&)[N]);

int main(int argc, char const *argv[]) {
    ::print<int, 3>(7);
    
    const size_t INDEX = 3;
    const int INT_NUMS[] {1, 2, 3, 4, 5};
    std::cout << "INT_NUMS[" << 3 << "] <= " << ::get(INT_NUMS, INDEX) << std::endl;

    std::cout << "average of INT_NUMS <= " << ::average(INT_NUMS) << std::endl;

    return 0;
}

template <typename T, size_t N> void print(const T& value) {
    for (size_t i{}; i < N; i++) {
        std::cout << value;
    }
    std::cout << std::endl;
}

template <typename T, size_t N> T get(const T(&data)[N], const size_t index) {
    return data[index];
}

template <typename T, size_t N> T average(const T(&data)[N]) {
    T result{};
    for (auto datum: data) {
        result += datum;   
    }

    return result / N;
}
