#include <iostream>
#include <format>
#include <string>

namespace {

    class Trie {
    public:
        static constexpr int SIZE{26};

    private:
        struct TrieNode {
            TrieNode* children[SIZE] = {nullptr};
            bool is_end{false};
        };

        TrieNode* root{nullptr};

    public:
        explicit Trie(): root(new TrieNode()) {}

        void insert(const std::string& word) const {
            TrieNode* node{root};
            for (const char c: word) {
                const int idx{c - START_SYMBOL};
                if (!node->children[idx]) {
                    node->children[idx] = new TrieNode();
                }
                node = node->children[idx];
            }

            node->is_end = true;
        }

        [[nodiscard]] bool search(const std::string& word) const {
            const TrieNode* node{find_node(word)};
            return node != nullptr && node->is_end;
        }

        [[nodiscard]] bool start_with(const std::string& prefix) const {
            return find_node(prefix) != nullptr;
        }

        static constexpr char START_SYMBOL{'a'};

    private:
        [[nodiscard]] TrieNode* find_node(const std::string& s) const {
            TrieNode* node{root};
            for (const char c: s) {
                const int idx{c - START_SYMBOL};
                if (!node->children[idx]) return nullptr;
                node = node->children[idx];
            }

            return node;
        }

        friend class Visitor;
    };

    class Visitor {
    public:
        void visit(const Trie& element) {
            append(element.root);
        }

        [[nodiscard]] std::string get_result() const {
            return result;
        }
    private:
        void append(const Trie::TrieNode* base_node, std::string offset = "") {
            for (int i{}; i < Trie::SIZE; ++i) {
                if (!base_node->children[i]) continue;

                result += std::format("{}{}\n", offset, static_cast<char>(Trie::START_SYMBOL + i));
                append(base_node->children[i], offset + " ");
            }
        }

        std::string result;
    };
}

int main() {

    const Trie trie;
    trie.insert("hello");
    trie.insert("world");

    Visitor visitor;
    visitor.visit(trie);

    std::cout << visitor.get_result() << std::endl;

    return 0;
}
