#include "backtrack_permutations.h"

#include <vector>
#include <string>
#include <iostream>
#include <format>

namespace backtrack_permutations {

void backtrack(std::vector<int>& nums,
                std::vector<int>& path,
                std::vector<bool>& used,
                std::vector<std::vector<int>>& result) {
    if (path.size() == nums.size()) {
        // путь полностью построен -> сохраняем копию
        result.push_back(path);
        return;
    }

    for (size_t i{0}; i < nums.size(); ++i) {
        if (used[i]) continue;

        used[i] = true;
        path.push_back(nums[i]);

        backtrack(nums, path, used, result);

        // откат: убираем последний выбранный элемент
        path.pop_back();
        // откат: помечаем элемент снова доступным
        used[i] = false;
    }
}

std::vector<std::vector<int>> permute(std::vector<int>& nums) {
    std::vector<std::vector<int>> result;
    std::vector<int> path;
    std::vector<bool> used(nums.size(), false);

    backtrack(nums, path, used, result);
    return result;
}

void backtrack_swap(std::vector<int>& nums, const int start, std::vector<std::vector<int>>& result) {
    if (start == static_cast<int>(nums.size())) {
        result.push_back(nums);
        return;
    }

    for (int i{start}; i < static_cast<int>(nums.size()); ++i) {
        std::swap(nums[start], nums[i]);
        backtrack_swap(nums, start + 1, result);
        // откат обмена
        std::swap(nums[start], nums[i]);
    }
}

void demo() {
    std::vector<int> nums0{1, 2, 3};
    const auto result0 = permute(nums0);

    std::vector<int> nums1{1, 2, 3};
    std::vector<std::vector<int>> result1;
    backtrack_swap(nums1, 0, result1);

    const auto print = [](const std::vector<std::vector<int>>& nums, const std::string &label) {
        std::cout << std::format("{} \n", label);
        for (const auto& vec: nums) {
            for (const auto& i: vec) {
                std::cout << std::format("{} ", i);
            }
            std::cout << '\n';
        }
        std::cout << "\n\n";
    };

    print(result0, "R0");
    print(result1, "R1");
}

}