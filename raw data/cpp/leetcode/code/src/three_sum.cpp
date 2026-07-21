#include "three_sum.h"

#include <iostream>
#include <format>
#include <algorithm>
#include <ranges>

namespace three_sum {

std::vector<std::vector<int>> get_three_sum(std::vector<int>& nums) {
    std::ranges::sort(nums);
    std::vector<std::vector<int>> result;
    int N{static_cast<int>(nums.size())};

    for (int i{}; i < N - 2; ++i) {
        // отсортировано: дальше только положительные -> сумма 0 невозможна
        if (nums[i] > 0) break;

        // пропуск дубликата первого элемента
        if (i > 0 && nums[i] == nums[i - 1]) continue;

        int left{i + 1};
        int right{N - 1};
        int target{ -nums[i] };

        while (left < right) {
            if (int sum{nums[left] + nums[right]}; sum == target) {
                result.push_back({nums[i], nums[left], nums[right]});

                // сдвигаем оба указателя и пропускаем дубликаты
                while (left < right && nums[left] == nums[left  + 1]) ++left;
                while (left < right && nums[right] == nums[right - 1]) --right;

                ++left;
                --right;
            } else if (sum < target) {
                ++left;
            } else {
                --right;
            }
        }
    }

    return result;
}

void demo() {
    std::vector<int> NUMBERS{-1, 0, 1, 2, -1, -4};
    for (const auto& result = get_three_sum(NUMBERS); auto& vec: result ) {
        std::string delimiter{""};
        std::cout << "[";
        for (auto& item: vec) {
            std::cout << std::format("{}{}", delimiter, item);
            delimiter = ", ";
        }
        std::cout << "]\n";
    }
}

}