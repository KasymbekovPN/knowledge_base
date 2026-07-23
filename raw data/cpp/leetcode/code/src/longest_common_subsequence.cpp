#include "longest_common_subsequence.h"

#include <iostream>
#include <format>
#include <string>
#include <vector>
#include <algorithm>

namespace longest_common_subsequence {

int calc_longest_common_subsequence(const std::string& text0, const std::string& text1) {
    std::cout << "calc_longest_common_subsequence\n";
    const int N{static_cast<int>(text0.size())};
    const int M{static_cast<int>(text1.size())};

    std::vector<std::vector<int>> dp(N + 1, std::vector<int>(M + 1, 0));
    auto print = [&dp, &text0, &text1](const int i, const int j) {
        std::cout << std::format("{}\n{}^\n{}\n{}^\n",
            text0,
            std::string(i, ' '),
            text1,
            std::string(j, ' '));
        for (const auto& vec: dp) {
            for (const auto& item: vec) {
                std::cout << std::format("{} ", item);
            }
            std::cout << '\n';
        }

        std::cout << "\n\n";
    };

    for (int i{1}; i <= N; ++i) {
        for (int j{1}; j <= M; ++j) {
            if (text0[i-1] == text1[j-1]) {
                dp[i][j] = dp[i-1][j-1] + 1;
            } else {
                dp[i][j] = std::max(dp[i-1][j], dp[i-1][j-1]);
            }
            // print(i, j + 1);
        }
    }

    return dp[N][M];
}

int calc_longest_common_subsequence_opt(const std::string& text0, const std::string& text1) {
    std::cout << "calc_longest_common_subsequence_opt\n";
    const int N{static_cast<int>(text0.size())};
    const int M{static_cast<int>(text1.size())};

    std::vector<int> prev(M+1, 0);
    std::vector<int> current(M+1, 0);
    const auto print = [&prev](const int i, const int j) {
        std::cout << std::format("{} {} [", i, j);
        for (const auto& item: prev) std::cout << std::format("{} ", item);
        std::cout << "]\n";
    };

    for (int i{1}; i <= N; ++i) {
        for (int j{1}; j <= M; ++j) {
            if (text0[i-1] == text1[j-1]) {
                current[j] = prev[j-1] + 1;
            } else {
                current[j] = std::max(prev[j], current[j-1]);
            }
            // print (i, j);
        }

        // текущая строка становится "предыдущей" для следующей итерации
        prev = current;
    }

    return prev[M];
}

std::string reconstruct_lcs(const std::string& text0, const std::string& text1, const std::vector<std::vector<int>>& dp) {
    std::cout << "reconstruct_lcs\n";
    std::string result;
    int i{static_cast<int>(text0.size())};
    int j{static_cast<int>(text1.size())};

    auto print = [&dp, &text0, &text1, &i, &j]() {
        std::cout << std::format("{}\n{}^\n{}\n{}^\n",
            text0,
            std::string(i, ' '),
            text1,
            std::string(j, ' '));
        for (const auto& vec: dp) {
            for (const auto& item: vec) {
                std::cout << std::format("{} ", item);
            }
            std::cout << '\n';
        }

        std::cout << "\n\n";
    };

    while (i > 0 && j > 0) {
        if (text0[i-1] == text1[j-1]) {
            result += text0[i-1];
            --i; --j;
        } else if (dp[i-1][j] >= dp[i][j-1]) {
            --i;
        } else {
            --j;
        }
        // print();
    }
    std::ranges::reverse(result);

    return  result;
}

void demo() {
    const std::string text0{"abcde"};
    const std::string text1{"ace"};

    std::cout << calc_longest_common_subsequence(text0, text1) << '\n';
    std::cout << calc_longest_common_subsequence_opt(text0, text1) << '\n';

    std::vector<std::vector<int>> dp = {
        {0, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 1, 0},
        {0, 1, 2, 1},
        {0, 1, 2, 2},
        {0, 1, 2, 3}
    };
    std::cout << reconstruct_lcs(text0, text1, dp) << '\n';
}

}