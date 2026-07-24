[[raw data/cpp/interview/_|<=]]

## N-Queens

**Условие:** дано число `n`. Расставить `n` ферзей на доске `n×n` так, чтобы ни один ферзь не бил другого (ни по горизонтали, ни по вертикали, ни по диагонали). Вернуть все различные расстановки (обычно в виде списка досок, представленных строками, или списка позиций).

### Идея

Классический backtracking с важной оптимизацией: поскольку в каждой строке может стоять **ровно один** ферзь (иначе они бьют друг друга по горизонтали), не нужно перебирать все `n²` клеток — достаточно идти по строкам одна за другой и для каждой строки перебирать, в какой из `n` колонок поставить ферзя. Это сразу снижает пространство перебора с `C(n², n)` до `n^n`, а с проверками — на практике значительно меньше.

Для быстрой проверки "бьёт ли новый ферзь кого-то по вертикали/диагоналям" держим три множества (или булевых массива):

- `cols` — какие колонки уже заняты;
- `diag1` — какие диагонали "слева-направо-вниз" (индекс `row - col`) заняты;
- `diag2` — какие диагонали "справа-налево-вниз" (индекс `row + col`) заняты.

Ключевое наблюдение: у всех клеток на одной диагонали `row - col` — постоянно, а на другой диагонали `row + col` — тоже постоянно. Это позволяет проверять диагональные конфликты за O(1) вместо явного обхода диагонали.

### Решение

```cpp
#include "nqueens.h"  
  
#include <vector>  
#include <unordered_set>  
#include <string>  
#include <iostream>  
#include <format>  
  
namespace nqueens {  
  
void backtrack(const int row,  
                const int n,  
                std::vector<int>& queens,  
                std::unordered_set<int>& cols,  
                std::unordered_set<int>& diag1,  
                std::unordered_set<int>& diag2,  
                std::vector<std::vector<std::string>>& result) {  
    if (row == n) {  
        // построить доску из queens и сохранить  
        std::vector<std::string> board(n, std::string(n, '.'));  
        for (int r{}; r < n; ++r) {  
            board[r][queens[r]] = 'Q';  
        }        result.push_back(std::move(board));  
        return;  
    }  
    for (int col{}; col < n; ++col) {  
        if (cols.contains(col) || diag1.contains(row - col) || diag2.contains(row + col)) {  
            continue; // клетка бьётся уже поставленным ферзём  
        }  
  
        // размещаем ферзя  
        queens[row] = col;  
        cols.insert(col);  
        diag1.insert(row - col);  
        diag2.insert(row + col);  
  
        backtrack(row + 1, n, queens, cols, diag1, diag2, result);  
  
        // откат  
        cols.erase(col);  
        diag1.erase(row - col);  
        diag2.erase(row + col);  
    }}  
  
std::vector<std::vector<std::string>> solve_nqueens(const int n) {  
    std::vector<std::vector<std::string>> result;  
    std::vector<int> queens(n, -1);  
    std::unordered_set<int> cols, diag1, diag2;  
    backtrack(0, n, queens, cols, diag1, diag2, result);  
  
    return result;  
}  
  
void backtrack_bitmask(const int row,  
                        const int n,  
                        const int cols,  
                        const int diag1,  
                        const int diag2,  
                        int& count) {  
    if (row == n) {  
        ++count;  
        return;  
    }  
    // доступные позиции в этой строке: биты, не занятые ни cols, ни diag1, ни diag2  
    int available{((1 << n) - n) & ~(cols | diag1 | diag2)};  
    while (available) {  
        // выделяем младший установленный бит  
        const int bit{available & (-available)};  
        available = -bit;  
  
        backtrack_bitmask(row + 1, n, cols | bit, (diag1 | bit) << 1, (diag2 | bit) >> 1, count);  
    }}  
  
void demo() {  
    // int count{};  
    // backtrack_bitmask(0, 4, 0, 0, 0, count);    // std::cout << std::format("count: {}\n", count);  
    for (const auto result = solve_nqueens(4);  
        const auto& vec : result) {  
        for (const auto& item : vec) {  
            std::cout << std::format("{} \n", item);  
        }        std::cout << '\n';  
    }}  
  
}
```

### Разбор

- `row` — номер текущей строки, для которой выбираем позицию ферзя; рекурсия идёт строго по возрастанию строк, поэтому конфликты "по горизонтали" исключены структурой алгоритма (в одной строке — ровно один вызов, один ферзь).
- `cols` — какие колонки уже заняты ферзями в предыдущих строках.
- `row - col` — инвариант диагонали, идущей "с северо-запада на юго-восток" (все клетки этой диагонали дают одинаковое значение `row - col`).
- `row + col` — инвариант диагонали "с северо-востока на юго-запад".
- Условие `continue` в цикле — если колонка или любая из двух диагоналей уже заняты, эта позиция не подходит, пропускаем её без углубления в рекурсию (это и есть pruning — отсечение заведомо бесперспективных веток).
- База `row == n` — все `n` ферзей расставлены без конфликтов, строим текстовое представление доски и сохраняем.
- Откат (`erase` после рекурсивного вызова) — освобождает колонку/диагонали, занятые текущим ферзём, перед тем как цикл попробует следующую колонку `col` в этой же строке.

### Пример (n=4)

```
row=0: пробуем col=0,1,2,3
  col=0: queens=[0], cols={0}, diag1={0}, diag2={0}
    row=1: col=0 занято(cols); col=1: diag1(1-1=0) занято -> пропуск
           col=2: cols нет, diag1(1-2=-1) нет, diag2(1+2=3) нет -> ставим
      queens=[0,2], row=2: 
        col=0 занято; col=1: diag2(2+1=3) занято(от row1); 
        col=2: diag1(2-2=0) занято(от row0); col=3: diag1(2-3=-1) занято(от row1)
        -> нет вариантов, откат
      откат ферзя row=1
    col=3: diag2(1+3=4) нет, diag1(1-3=-2) нет -> ставим
      queens=[0,3], row=2:
        col=1: diag1(2-1=1) нет, diag2(2+1=3) нет -> ставим
          queens=[0,3,1], row=3:
            col=2: cols нет, diag1(3-2=1) занято(от row2 col1: 2-1=1) -> пропуск
            остальные тоже конфликтуют -> откат
        col=2: diag2(2+2=4) занято(от row1: 1+3=4) -> пропуск
        -> откат из row=1(col=3)
    -> откат из col=0 (row=0), пробуем col=1
  ... (аналогично для col=1,2,3 в row=0)

Итог для n=4: 2 решения
[".Q..","...Q","Q...","..Q."] и ["..Q.","Q...","...Q",".Q.."]
```

### Сложность

- Время: **O(n!)** в худшем случае (хотя pruning на практике сильно сокращает реальное число посещённых узлов дерева перебора) — точная асимптотика сложно выражается простой формулой, но `n!` — стандартная верхняя оценка для этого класса задач с ограничениями по строкам/колонкам.
- Память: **O(n)** — глубина рекурсии, размер `queens`, `cols`, `diag1`, `diag2` (не считая памяти под сохранённые решения).

### Более быстрая версия: битовые маски вместо hash-set

Для больших `n` (когда важна константа по времени) используют битовые маски `int`/`long long` вместо трёх `unordered_set`, что даёт заметное ускорение за счёт битовых операций вместо хеширования:

Разбор: `cols`, `diag1`, `diag2` — теперь битовые маски, где бит `j` означает "колонка `j` под боем". При переходе на следующую строку `diag1` сдвигается влево (`<<1`), `diag2` — вправо (`>>1`), что естественным образом моделирует смещение диагоналей на одну строку вниз без явного пересчёта индексов `row±col`. `bit & (-bit)` — классический трюк выделения младшего установленного бита в двоичном представлении.

### Частые вариации

- **N-Queens II** — то же самое, но нужно вернуть только **количество** решений, а не сами доски — экономит память (не нужно строить и хранить текстовые представления), обычно решается именно битовой версией.
- **Sudoku Solver** — похожий backtracking с ограничениями (строка/колонка/блок 3×3), но с более сложной проверкой допустимости и большим пространством значений (1-9 вместо 0/1 "ферзь есть/нет").
- **Word Search** — backtracking по сетке, но ограничения другие (нельзя проходить через уже посещённую клетку в текущем пути, а не "не бить по диагонали").

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
- [ ] backtracking - Combination Sum
- [ ] other - Binary Search
- [ ] other - Search in Rotated Sorted Array
- [ ] other - Trapping Rain Water
- [ ] other - Merge K Sorted Lists
- [ ] other - Design a Trie (Prefix Tree)


