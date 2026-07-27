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
    }}  
  
const TrieNode* Trie::find_node(const std::string& word) const {  
    const TrieNode* node{root_.get()};  
  
    for (const auto& c: word) {  
        const int IDX{calculate_idx(c)};  
        if (!node->children[IDX]) {  
            return nullptr;  
        }        node = node->children[IDX].get();  
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
