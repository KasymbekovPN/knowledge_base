#include <algorithm>
#include <iostream>
#include <format>
#include <unordered_map>

namespace {
    class AutocompleteSystem {
        struct TrieNode {
            std::unordered_map<char, TrieNode*> children;
            std::unordered_map<std::string, int> counts; // полный запрос -> частота, хранится в каждом узле на пути
        };

        TrieNode* root{nullptr};
        TrieNode* current{nullptr};
        std::string current_input;

    public:

        explicit AutocompleteSystem(const std::vector<std::string>& sentences, const std::vector<int>& times) {
            root = new TrieNode();
            current = root;
            for (int i{}; i < sentences.size(); ++i) {
                insert(sentences[i], times[i]);
            }
        }

        void insert(const std::string& sentence, const int time) {
            TrieNode* node{root};
            for (char c : sentence) {
                if (!node->children.contains(c)) {
                    node->children[c] = new TrieNode();
                }
                node = node->children[c];
                node->counts[sentence] += time;
            }
        }

        std::vector<std::string> input(const char c) {
            if (c == '#') {
                insert(current_input, 1);
                current_input.clear();
                current = root;

                return {};
            }

            current_input += c;

            if (current && current->children.contains(c)) {
                current = current->children[c];
            } else {
                // префикс больше не существует в истории
                current = nullptr;
                return {};
            }

            // top-3 из текущего узла — здесь ровно top-K selection из первого разбора
            std::vector<std::pair<std::string, int>> candidates(
                current->counts.begin(), current->counts.end());

            std::partial_sort(
                candidates.begin(),
                candidates.begin() + std::min(static_cast<size_t>(3), candidates.size()),
                candidates.end(),
                [](auto& a, auto& b) {
                    if (a.second != b.second) return a.second > b.second;
                    return a.first < b.first;
                });

            std::vector<std::string> results;
            for (int i{}; i < std::min(static_cast<size_t>(3), candidates.size()); ++i) {
                results.push_back(candidates[i].first);
            }

            return results;
        }
    };

}

int main() {
    // Исторические запросы и их частоты (times[i] — сколько раз запрос sentences[i] уже вводился ранее)
    std::vector<std::string> sentences = {
        "i love you",
        "island",
        "iroman",
        "i love leetcode"
    };
    std::vector<int> times = {5, 3, 2, 2};

    // Конструктор сразу строит Trie из этих исторических данных
    AutocompleteSystem obj(sentences, times);

    // Симулируем посимвольный ввод пользователем "i love c"
    std::vector<char> typed = {'i', ' ', 'l', 'o', 'v', 'e', ' ', 'c', '#'};

    for (char c : typed) {
        std::vector<std::string> suggestions = obj.input(c);

        std::cout << "After input '" << c << "': ";
        if (suggestions.empty()) {
            std::cout << "(no one tip)";
        } else {
            for (const auto& s : suggestions) {
                std::cout << "[" << s << "] ";
            }
        }
        std::cout << std::endl;
    }

    return 0;
}
