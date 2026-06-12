#include <iostream>

namespace console {
    namespace messages {
        const std::string hello {"Hello"};
        const std::string welcome {"Welcome"};
        const std::string goodbye {"Good bye"};
    }

    void print(const std::string&);
    void print_default();
}

int main(int argc, char const *argv[]) {
    console::print_default();
    console::print(console::messages::hello);
    console::print(console::messages::goodbye);

    return 0;
}

void console::print(const std::string& text) {
    std::cout << text << std::endl;
}

void console::print_default() {
    print(messages::welcome);
}
