#include <iostream>

namespace {
    struct Empty {};

    struct WithoutAttr {
        Empty e;
        int x;
    };

    struct WithAttr {
        // [[no_unique_address]] Empty e;
        [[msvc::no_unique_address]] Empty e;
        int x;
    };
}

int main() {
    std::cout << "WithoutAttr: " << sizeof(WithoutAttr) << std::endl;
    std::cout << "WithAttr: " << sizeof(WithAttr) << std::endl;

    return 0;
}
