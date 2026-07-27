#pragma once

#include <memory>
#include <string>

namespace trie_prefix_tree {
    class TrieNode;

    class Trie {
    public:
        explicit Trie();
        void insert(const std::string&);
        bool search(const std::string&) const;
        bool starts_with(const std::string&) const;
        void print(TrieNode*, const int);
    private:
        const TrieNode* find_node(const std::string&) const;

        std::unique_ptr<TrieNode> root_;
    };

    void demo();
}