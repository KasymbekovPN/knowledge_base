#include <iostream>
#include <string>
#include <deque>

struct Result {
    bool success;
    int value;

    Result(bool success = false, int value = 0):
        success{success},
        value {value} {}
    std::string to_string() const {
        return "{success: " + std::to_string(success) + 
                ", value: " + std::to_string(value) + "}";
    };
};

Result _at(const std::deque<int>&, unsigned);

int main(int argc, char const *argv[]) {
    std::deque<int> deq {1, 2, 3};
    for (unsigned i{}; i < 5; i++) {
        std::cout << _at(deq, i).to_string() << std::endl;
    }
    

    return 0;
}

Result _at(const std::deque<int>& deq, unsigned index) {
    try {
        return Result{true, deq.at(index)};
    } catch(const std::exception& e) {
        return Result{};
    }
}
