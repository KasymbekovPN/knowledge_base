#include "combination_sum.h"

#include <vector>
#include <algorithm>
#include <iostream>
#include <format>

namespace combination_sum {

void backtrace(const std::vector<int>& candidates,
               const int target,
               const int start,
               std::vector<int>& path,
               std::vector<std::vector<int>>& result) {
    if (target == 0) {
        // сумма точно набрана -> сохраняем комбинацию
        result.push_back(path);
        return;
    }

    for (int i{start}; i < static_cast<int>(candidates.size()); ++i) {
        if (candidates[i] > target) {
            // массив отсортирован -> все дальнейшие элементы тоже слишком большие
            break;
        }

        path.push_back(candidates[i]);
        // i, не i+1 — повтор разрешён
        backtrace(candidates, target - candidates[i], i, path, result);
        // откат
        path.pop_back();
    }
}

std::vector<std::vector<int>> combination_sum(const std::vector<int>& candidates, const int target) {
    std::vector<std::vector<int>> result;
    std::vector<int> path;
    backtrace(candidates, target, 0, path, result);

    return result;
}

void demo() {
    const std::vector<int> SORTED_CANDIDATES{2,3,6,7};
    constexpr  int TARGET{7};

    for (const auto& result = combination_sum(SORTED_CANDIDATES, TARGET);
        const auto& vec: result) {
        for (const auto& item: vec) {
            std::cout << std::format("{} ", item);
        }
        std::cout << '\n';
    }

}
}