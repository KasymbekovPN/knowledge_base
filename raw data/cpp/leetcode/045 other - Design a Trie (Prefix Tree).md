[[raw data/cpp/interview/_|<=]]

## Design a Trie (Prefix Tree)

**Условие:** реализовать структуру данных Trie (префиксное дерево) с операциями:

- `insert(word)` — добавить слово в trie.
- `search(word)` — проверить, есть ли **точно такое** слово в trie.
- `startsWith(prefix)` — проверить, есть ли в trie хотя бы одно слово, начинающееся с данного префикса.

### Идея

Trie — дерево, где каждый узел представляет один символ, а путь от корня до узла представляет некоторый префикс. Каждый узел хранит массив/map указателей на дочерние узлы (по одному на каждый возможный следующий символ) и флаг "является ли путь до этого узла концом целого слова".

Ключевое преимущество перед hash-set строк: вставка, поиск слова и поиск по префиксу — все выполняются за **O(L)**, где `L` — длина слова/префикса, **независимо от количества слов**, уже хранящихся в trie, и общие префиксы физически переиспользуют одни и те же узлы (экономия памяти на похожих словах).

### Решение

```cpp
#include <string>
#include <array>
#include <memory>

class TrieNode {
public:
    std::array<std::unique_ptr<TrieNode>, 26> children;
    bool isEndOfWord = false;
};

class Trie {
public:
    Trie() : root_(std::make_unique<TrieNode>()) {}

    void insert(const std::string& word) {
        TrieNode* node = root_.get();

        for (char c : word) {
            int idx = c - 'a';
            if (node->children[idx] == nullptr) {
                node->children[idx] = std::make_unique<TrieNode>();
            }
            node = node->children[idx].get();
        }

        node->isEndOfWord = true;
    }

    bool search(const std::string& word) const {
        const TrieNode* node = findNode(word);
        return node != nullptr && node->isEndOfWord;
    }

    bool startsWith(const std::string& prefix) const {
        return findNode(prefix) != nullptr;
    }

private:
    const TrieNode* findNode(const std::string& s) const {
        const TrieNode* node = root_.get();

        for (char c : s) {
            int idx = c - 'a';
            if (node->children[idx] == nullptr) {
                return nullptr; // путь оборвался -> такого префикса нет
            }
            node = node->children[idx].get();
        }

        return node; // дошли до конца s -> возвращаем узел (не проверяем isEndOfWord здесь)
    }

    std::unique_ptr<TrieNode> root_;
};
```

### Разбор

- `TrieNode::children` — фиксированный массив из 26 указателей (предполагаем алфавит `'a'`..`'z'`); `children[idx] != nullptr` означает "есть ребёнок по символу `idx`".
- `isEndOfWord` — критически важный флаг: он различает "узел, через который проходит путь к более длинному слову" от "узел, который сам является концом вставленного слова". Например, если вставлены `"app"` и `"apple"`, узел на конце `"app"` должен иметь `isEndOfWord = true`, даже несмотря на то, что дерево продолжается дальше к `"apple"`.
- `std::unique_ptr<TrieNode>` — управление памятью автоматическое (RAII): при разрушении `Trie` вся цепочка `unique_ptr` рекурсивно освобождает все узлы без ручных `delete` и без утечек.
- `findNode` — общий вспомогательный метод для `search` и `startsWith`: оба сводятся к "дойти по пути символов до конца строки, если получилось — вернуть узел". Разница между ними только в финальной проверке: `search` дополнительно требует `isEndOfWord == true`, а `startsWith` — нет (достаточно, что путь вообще существует).
- `c - 'a'` — стандартный трюк перевода символа в индекс массива 0..25 (предполагает нижний регистр латиницы; для более широкого алфавита понадобится `unordered_map<char, unique_ptr<TrieNode>>` вместо фиксированного массива).

### Пример

```
Trie trie;
trie.insert("apple");

trie.search("apple");   // true — слово "apple" было вставлено целиком
trie.search("app");     // false — "app" не вставлялось как отдельное слово,
                         //         хотя путь до узла 'p' существует (isEndOfWord=false там)
trie.startsWith("app"); // true — путь "a"->"p"->"p" существует в дереве
trie.insert("app");
trie.search("app");     // true — теперь "app" тоже явно вставлено (isEndOfWord=true на этом узле)
```

**Визуализация дерева после `insert("apple")` и `insert("app")`:**

```
root
 └─ a
     └─ p
         └─ p  (isEndOfWord = true, после insert("app"))
             └─ l
                 └─ e  (isEndOfWord = true)
```

### Сложность

- `insert`: время **O(L)**, где `L` — длина слова; память — до O(L) новых узлов (меньше, если часть пути уже существовала благодаря общему префиксу с ранее вставленными словами).
- `search` / `startsWith`: время **O(L)** — проход по дереву на глубину `L`.
- Память в целом: **O(суммарное количество символов во всех уникальных префиксах)** — в худшем случае (если слова не имеют общих префиксов) это O(суммарная длина всех слов), но на практике часто заметно меньше за счёт переиспользования общих путей.

### Частые вариации

- **Add and Search Word (Word Dictionary)** — `search` должен поддерживать wildcard `.` (означает "любой символ") → решается через DFS/backtracking по всем детям узла, когда встречается `.`, вместо прямого индексирования по одному конкретному символу.
- **Word Search II** — поиск нескольких слов из словаря сразу на 2D-сетке символов → строится Trie из всех слов словаря, затем DFS по сетке с одновременным движением по Trie, что позволяет искать все слова за один общий обход сетки вместо отдельного поиска каждого слова.
- **Replace Words** — для каждого слова в предложении найти кратчайший "корень" (root) из словаря, которым можно его заменить → Trie строится из корней, поиск — обход по Trie, пока не встретится `isEndOfWord = true`.
- **Longest Word in Dictionary** — найти самое длинное слово, которое можно построить посимвольно, добавляя по одной букве, где каждый промежуточный префикс — тоже слово из словаря → DFS/BFS по Trie с проверкой `isEndOfWord` на каждом шаге пути.

### Частый доп. вопрос: "почему `std::array<unique_ptr<TrieNode>, 26>`, а не `unordered_map<char, TrieNode*>`?"

Фиксированный массив даёт O(1) доступ по индексу без хеширования и без overhead на хранение самих ключей-символов — быстрее и компактнее при известном небольшом алфавите (26 латинских букв). `unordered_map` предпочтительнее, если алфавит большой и разреженный (например, Unicode) — тогда массив на все возможные символы был бы избыточно большим и в основном пустым.

---
---
---

- [x] Array/String - Two Sum (2026.07.18)
- [x] Array/String - Best Time to Buy and Sell Stock (2026.07.18)
- [x] Array/String - Maximum Subarray (Kadane's algorithm) (2026.07.18)
- [x] Array/String - Merge Intervals (2026.07.18)
- [x] Array/String - Product of Array Except Self (2026.07.19)
- [x] Array/String - Longest Substring Without Repeating Characters (2026.07.19)
- [x] Array/String - Group Anagrams (2026.07.19)
- [x] Array/String - Valid Parentheses (2026.07.19)
- [x] Array/String - Container With Most Water (2026.07.19)
- [x] Array/String - 3Sum (2026.07.19)
- [x] LinkedList - Reverse Linked List (2026.07.19)
- [x] LinkedList - Detect Cycle in Linked List (Floyd's) (2026.07.19)
- [x] LinkedList - Merge Two Sorted Lists (2026.07.21)
- [x] LinkedList - Remove Nth Node From End of List (2026.07.21)
- [x] LinkedList - LRU Cache (список + hash map) (2026.07.21)
- [x] Trees/Graphs - Maximum Depth of Binary Tree (2026.07.21)
- [x] Trees/Graphs - Validate Binary Search Tree (2026.07.21)
- [x] Trees/Graphs - Binary Tree Level Order Traversal (2026.07.21)
- [x] Trees/Graphs - Lowest Common Ancestor of a BST/BT (2026.07.21)
- [x] Trees/Graphs - Serialize and Deserialize Binary Tree (2026.07.22)
- [x] Trees/Graphs - Number of Islands (BFS/DFS) (2026.07.22)
- [x] Trees/Graphs - Clone Graph (2026.07.22)
- [x] Trees/Graphs - Course Schedule (topological sort) (2026.07.22)
- [x] Trees/Graphs - Word Ladder (BFS) (2026.07.22)
- [x] Dynamic - Climbing Stairs (2026.07.22)
- [x] Dynamic - Coin Change (2026.07.22)
- [x] Dynamic - Longest Common Subsequence (2026.07.23)
- [x] Dynamic - Longest Increasing Subsequence (2026.07.23)
- [x] Dynamic - Edit Distance (2026.07.23)
- [x] Dynamic - House Robber (2026.07.23)
- [x] Dynamic - Word Break (2026.07.23)
- [x] Dynamic - 0/1 Knapsack (2026.07.23)
- [x] head/stack/queue - Min Stack (2026.07.23)
- [x] head/stack/queue - Kth Largest Element in an Array (2026.07.24)
- [x] head/stack/queue - Top K Frequent Elements (2026.07.24)
- [x] head/stack/queue - Sliding Window Maximum (2026.07.24)
- [x] backtracking - Permutations (2026.07.24)
- [x] backtracking - Subsets (2026.07.24)
- [x] backtracking - N-Queens (2026.07.24)
- [x] backtracking - Combination Sum (2026.07.25)
- [x] other - Binary Search (2026.07.25)
- [x] other - Search in Rotated Sorted Array (2026.07.25)
- [x] other - Trapping Rain Water (2026.07.25)
- [x] other - Merge K Sorted Lists (2026.07.25)
- [ ] other - Design a Trie (Prefix Tree)


