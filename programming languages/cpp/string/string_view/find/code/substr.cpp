#include <iostream>
#include <string>
#include <string_view>

void _test_substr(const std::string_view&, const size_t, const size_t);

int main() {
    const std::string sv {"Hello"};
    for (size_t i {}; i < 5; i++) {
        _test_substr(sv, 3, i);
    }

    return 0;
}

void _test_substr(const std::string_view& sv, const size_t pos, const size_t len) {
    std::cout
        << "[" << pos << ", " << len << "] "
        << sv.substr(pos, len) << std::endl;
}
