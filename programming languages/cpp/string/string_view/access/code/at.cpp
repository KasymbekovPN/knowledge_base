#include <iostream>
#include <string>
#include <string_view>

void _test_at(const std::string_view&, size_t);

int main() {
    const std::string_view sv {"ABC"};
    for (size_t i {}; i <= sv.size(); i++) {
        _test_at(sv, i);
    }
    
    return 0;
}

void _test_at(const std::string_view& sv, size_t i) {
    try {
        std::cout << sv.at(i) << std::endl;
    } catch(const std::out_of_range& e) {
        std::cerr << e.what() << std::endl;
    }
}
