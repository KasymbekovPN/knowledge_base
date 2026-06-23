module;

#include <iostream>

export module templates;

export template<typename T>
T max(T _a, T _b) {
    return _a > _b ? _a : _b;
}

export template <typename T>
class Stack {
private:
    T data[100];
    int top{};
public:
    void push(T _value) { data[top++] = _value; }
    T pop() { return data[--top]; }
    void print() {
        std::string delimiter{""};
        std::cout << "[";
        for (int i{}; i < top; ++i) {
            std::cout << delimiter << i;
            delimiter = ", ";
        }
        std::cout << "]\n";
    }
};

export template<typename T>
constexpr T zero = T{};
