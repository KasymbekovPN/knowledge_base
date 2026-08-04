#include <iostream>

namespace {
    void process(const int value) {
        switch (value) {
            case 1:
                std::cout << "one/n";
                [[fallthrough]];
            case 2:
                std::cout << "one or two\n";
                break;
            case 3:
                std::cout << "three\n";
            case 4:
                std::cout << "three or four\n";
                break;
            default:
                std::cout << "other\n";
                break;
        }
    }
}

int main() {
    for (int i{1}; i <= 4; ++i) {
        process(i);
    }
}
