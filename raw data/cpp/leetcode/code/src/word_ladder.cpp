#include "word_ladder.h"

#include <queue>
#include <vector>
#include <string>
#include <unordered_set>
#include <iostream>
#include <format>

namespace word_ladder {

int ladder_len(const std::string& begin_word,
               const std::string& end_word,
               const std::vector<std::string>& word_list) {

    std::unordered_set<std::string> dict(word_list.begin(), word_list.end());
    if (!dict.contains(end_word)) return 0;

    std::queue<std::string> q;
    q.push(begin_word);
    dict.erase(begin_word);

    int steps{1};
    while (!q.empty()) {
        const int level_size{static_cast<int>(q.size())};

        for (int i{}; i < level_size; ++i) {
            std::string word{q.front()};
            q.pop();

            if (word == end_word) return steps;

            for (size_t pos{0}; pos < word.size(); ++pos) {
                const char original{word[pos]};

                for (char c{'a'}; c <= 'z'; ++c) {
                    if (c == original) continue;

                    word[pos] = c;

                    if (dict.contains(word)) {
                        // помечаем как посещённое (не заходить снова)
                        dict.erase(word);
                        q.push(word);
                    }
                }
                // восстанавливаем слово перед следующей позицией
                word[pos] = original;
            }
        }

        ++steps;
    }

    return 0;
}

void demo() {
    const std::string begin_word{"hit"};
    const std::string end_word{"cog"};
    const std::vector<std::string> word_list{"hot","dot","dog","lot","log","cog"};

    std::cout << std::format("LEN: {}\n", ladder_len(begin_word, end_word, word_list));
}

}
