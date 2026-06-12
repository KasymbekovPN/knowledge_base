#include <iostream>

void hello(std::string);

int main(int argc, char const *argv[]) {
    void (*message_0)(std::string) {nullptr};
    message_0 = hello;
    void (*message_1)(std::string) = hello;
    void (*message_2)(std::string) {hello};
    auto message_3 = hello;
    auto message_4 {hello};
    auto* message_5 {hello};
    auto message_6 {&hello};

    message_0("message_0");
    message_1("message_1");
    message_2("message_2");
    message_3("message_3");
    message_4("message_4");
    message_5("message_5");
    message_6("message_6");

    return 0;
}

void hello(std::string key) {
    std::cout << "hello " << key << " !!!" << std::endl;
}
