#include "trie_prefix_tree.h"

#include <memory>
#include <array>
#include <string>
#include <vector>
#include <iostream>
#include <format>

namespace trie_prefix_tree {

static constexpr char START_CHAR{'a'};

static int calculate_idx(const char c) {
    return c - START_CHAR;
}

class TrieNode {
public:
    static constexpr int SIZE{26};
    std::array<std::unique_ptr<TrieNode>, SIZE> children;
    bool is_end_of_word{false};
};

Trie::Trie(): root_{std::make_unique<TrieNode>()} {}

void Trie::insert(const std::string& word) {
    TrieNode* node{root_.get()};

    for (const auto& c: word) {
        const int IDX{calculate_idx(c)};
        if (!node->children[IDX]) {
            node->children[IDX] = std::make_unique<TrieNode>();
        }

        node = node->children[IDX].get();
    }

    node->is_end_of_word = true;
}

bool Trie::search(const std::string& word) const {
    const TrieNode* node{find_node(word)};
    return node && node->is_end_of_word;
}

bool Trie::starts_with(const std::string& word) const {
    return find_node(word) != nullptr;
}

void Trie::print(TrieNode* node, const int offset) {
    TrieNode* pointer = node ? node : root_.get();
    for (int i{}; i < 26; ++i) {
        if (!pointer->children[i]) continue;
        std::cout << std::format("{}{}\n", std::string(offset, ' '), static_cast<char>(i + START_CHAR));
        print(pointer->children[i].get(), offset + 1);
    }
}

const TrieNode* Trie::find_node(const std::string& word) const {
    const TrieNode* node{root_.get()};

    for (const auto& c: word) {
        const int IDX{calculate_idx(c)};
        if (!node->children[IDX]) {
            return nullptr;
        }
        node = node->children[IDX].get();
    }

    return node;
}

void demo() {
    const std::vector<std::string> WORDS{"hello", "world"};

    Trie trie;
    for (const auto& word : WORDS) {
        trie.insert(word);
    }

    trie.print(nullptr, 0);
}
}
