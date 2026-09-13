
## Суть задачи

Реализовать структуру Trie (префиксное дерево) с операциями:

- `insert(word)` — добавить слово
- `search(word)` — проверить, есть ли точное слово
- `startsWith(prefix)` — проверить, есть ли хоть одно слово с таким префиксом

## Идея структуры

Trie — дерево, где каждый узел представляет один символ, а путь от корня до узла — это префикс. Общие префиксы у разных слов "склеиваются" в общие ветки — в этом и экономия по сравнению с хранением слов как отдельных строк в hash-set.

Каждый узел хранит:

- Массив/map указателей на детей (по одному на возможный следующий символ)
- Флаг "здесь заканчивается слово" (`isEnd`)

```cpp
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

            static void delete_it(const TrieNode* node) {
                if (!node) return;
                for (int i{}; i < SIZE; ++i) delete_it(node->children[i]);
            }
        };

        TrieNode* root{nullptr};

    public:
        explicit Trie(): root(new TrieNode()) {}

        ~Trie() {
            TrieNode::delete_it(root);
        }

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

```

Сложность: `insert`/`search`/`startsWith` — все `O(L)`, где L — длина слова/префикса, **независимо от того, сколько слов уже в Trie**. Это и есть ключевое преимущество над hash-set для операций с префиксами: hash-set даёт `O(1)` на точный поиск, но не умеет отвечать на "есть ли слова с таким префиксом" быстрее чем `O(n·L)` перебором.

Компромисс — память: массив из 26 указателей на узел, даже если реально используется 1-2 ребёнка, — это лишние 26×8 байт на узел (на 64-битной системе — 208 байт впустую на разреженных узлах). На практике для больших словарей чаще используют `unordered_map<char, TrieNode*>` (экономия памяти, но чуть дороже по константе на доступ) или более компактные представления — об этом ниже.

## Параллель с поисковой платформой

Trie — фундаментальная структура для двух вещей, которые прямо в вашей вакансии:

1. **Автодополнение запросов** (это отдельно вынесено в LC 642, разберём следующим).
2. **Term dictionary инвертированного индекса.** В классических поисковых системах (Lucene и подобные) термы (слова) хранятся не в hash-таблице, а в структуре, похожей на trie или FST (finite state transducer) — потому что нужно быстро находить не только точный терм, но и **диапазон термов по префиксу** (для wildcard-запросов вроде `run*`), а также эффективно сжимать словарь, где много общих префиксов (что типично для естественного языка — множество слов с одинаковым корнем).

## На что готовиться на собеседовании

- **"Как оптимизировать память на реальных данных (миллионы термов)?"** — ожидаемый ответ: сжатие через **radix tree / Patricia trie** (склеивание цепочек узлов с одним ребёнком в один узел с строкой вместо символа), либо переход на **FST** (Lucene использует именно FST для term dictionary — компактнее, чем trie, за счёт разделения не только префиксов, но и суффиксов).
- **"А если нужно искать не точный префикс, а с опечатками (fuzzy search)?"** — здесь всплывает **Levenshtein automaton** поверх trie/FST — тема на стыке структур данных и NLP, вероятно, upper bound сложности вопроса для этой роли, но концептуально стоит знать, что это существует и зачем.
- **"Как делать Trie потокобезопасным для конкурентного чтения при обновлении индекса?"** — снова та самая explicit тема "lock-free" из вакансии: в реальных поисковых системах индекс обычно **immutable** (read-only сегменты), а обновления идут через создание нового сегмента и atomic swap указателя на него, а не через мутацию существующего Trie под локами. Это важный архитектурный паттерн, который стоит явно назвать, если разговор зайдёт в эту сторону — потому что "просто добавить mutex на insert" — это неправильный ответ для системы с высоким QPS на чтение.