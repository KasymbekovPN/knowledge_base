#include "sliding_window_max.h"

#include <iostream>
#include <ostream>
#include <vector>
#include <queue>

namespace sliding_window_max {

std::vector<int> max_sliding_window(const std::vector<int>& nums, const int k) {
    // хранит индексы, значения по ним убывают от начала к концу
    std::deque<int> dq;
    std::vector<int> result;

    const int N{static_cast<int>(nums.size())};
    result.reserve(N - k + 1);

    for (int i{}; i < N; ++i) {
        // убираем с конца все индексы с меньшим или равным значением — они бесполезны
        while (!dq.empty() && nums[dq.back()] <= nums[i]) dq.pop_back();
        dq.push_back(i);

        // убираем с начала индексы, вышедшие из текущего окна [i-k+1, i]
        if (dq.front() <= i - k) dq.pop_front();

        // окно ещё не набрало полный размер k — максимум пока не фиксируем
        if (i >= k - 1) result.push_back(nums[dq.front()]);
    }
    return result;
}

void demo() {
    constexpr int K{3};
    const std::vector<int> NUMS{1,3,-1,-3,5,3,6,7};

    for (const auto& num : max_sliding_window(NUMS, K)) {
        std::cout << num << std::endl;
    }
}

}