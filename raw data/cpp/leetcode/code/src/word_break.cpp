#include "word_break.h"

#include <vector>
#include <string>
#include <unordered_set>
#include <iostream>

namespace word_break {

bool word_break(const std::string& s, const std::vector<std::string>& word_dict) {
    const std::unordered_set<std::string> DICT(word_dict.begin(), word_dict.end());
    const int N{static_cast<int>(s.size())};

    std::vector<bool> dp(N+1, false);
    dp[0] = true;

    for (int i{1}; i <= N; ++i) {
        for (int j{0}; j <= i; ++j) {
            if (dp[j] && DICT.contains(s.substr(j, i - j))) {
                dp[i] = true;
                // нашли хотя бы один способ разбить префикс до i -> дальше не ищем
                break;
            }
        }
    }

    return dp[N];
}

void demo() {
    const std::string S{"leetcode"};
    const std::vector<std::string> DICT{"leet", "code"};
    std::cout
        << "word_break : "
        << std::boolalpha
        << word_break(S, DICT)
        << std::noboolalpha
        << std::endl;
}

}