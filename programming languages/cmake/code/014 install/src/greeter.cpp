#include "mylib/greeter.hpp"

#include <format>

std::string greet(const std::string& name) {
    return std::format("Hello, {}!", name);
}
