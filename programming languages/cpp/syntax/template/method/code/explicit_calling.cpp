#include <iostream>

template<typename T> T add(T, T);

int main(int argc, char const *argv[]) {
    std::cout << "implicit: " << add(12.3, 45.6) << std::endl; 
    std::cout << "explicit: " << add<int>(12.3, 45.6) << std::endl; 

    return 0;
}

template<typename T> T add(T a, T b) {
    return a + b;
}
