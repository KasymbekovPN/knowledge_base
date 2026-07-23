#include "edit_distance.h"

#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <format>

namespace edit_distance {

int min_distance(const std::string& word1, const std::string& word2) {
    const int N{static_cast<int>(word1.size())};
    const int M{static_cast<int>(word2.size())};

    std::vector<std::vector<int>> dp(N+1, std::vector<int>(M+1, 0));
    for (int i{}; i <= N; ++i) dp[i][0] = i;
    for (int j{}; j <= M; ++j) dp[0][j] = j;

    const auto print = [&dp](const int _i, const int _j) {
        std::cout << std::format("{} {}\n", _i, _j);
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
            if (word1[i-1] == word2[j-1]) {
                dp[i][j] = dp[i-1][j-1];
            } else {
                dp[i][j] = 1 + std::min({
                    dp[i-1][j-1], // замена
                    dp[i-1][j], // удаление
                    dp[i][j-1] // вставка
                });
            }
            // print(i, j);
        }
    }

    return dp[N][M];
}

int min_distance_opt(const std::string& word1, const std::string& word2) {
    const int N{static_cast<int>(word1.size())};
    const int M{static_cast<int>(word2.size())};

    std::vector<int> prev(M+1);
    std::vector<int> current(M+1);
    for (int j{}; j <= M; ++j) prev[j] = j;

    for (int i{1}; i <= N; ++i) {
        current[0] = i;
        for (int j{1}; j <= M; ++j) {
            if (word1[i - 1] == word2[j - 1]) {
                current[j] = prev[j-1];
            } else {
                current[j] = 1 + std::min({
                    current[j-1],
                    current[j],
                    current[j-1]});
            }
        }
        prev = current;
    }

    return prev[M];
}

void demo() {
    const std::string word1{"horse"};
    const std::string word2{"ros"};
    std::cout << std::format("min_distance {}\n", min_distance(word1, word2));
    std::cout << std::format("min_distance_opt {}\n", min_distance_opt(word1, word2));
}

}