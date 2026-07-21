#include "product_except_self.h"

#include <iostream>
#include <format>

namespace product_except_self {

static void print_vector(const std::vector<int>& v, std::string&& label) {
    std::cout << std::format("[{}] (", label);
    std::string delimiter;
    for (const auto& i : v) {
        std::cout << std::format("{}{}", delimiter, i);
        delimiter = ", ";
    }
    std::cout << ")\n";
}

std::vector<int> make_product(const std::vector<int>& nums) {
    const int N{static_cast<int>(nums.size())};
    std::vector<int> answer(N, 1);

    // проход слева направо: answer[i] = произведение nums[0..i-1]
    int prefix{1};
    for (int i{}; i < N; ++i) {
        answer[i] = prefix;
        prefix *= nums[i];
    }

    // проход справа налево: домножаем на произведение nums[i+1..n-1]
    int suffix{1};
    for (int i{N-1}; i >= 0; --i) {
        answer[i] *= suffix;
        suffix *= nums[i];
    }

    return answer;
}

void demo() {
    const std::vector<int> NUMS = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    auto result0 = make_product(NUMS);
    print_vector(result0, "V0");
}

}
