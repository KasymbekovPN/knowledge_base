[[raw data/cpp/interview/_|<=]]

## Coin Change

**Условие:** дан массив номиналов монет `coins` (неограниченное количество каждого номинала) и целевая сумма `amount`. Найти минимальное количество монет, чтобы набрать эту сумму. Если это невозможно — вернуть -1.

### Идея

Жадный подход (брать всегда самую крупную подходящую монету) **не работает в общем случае** — классический контрпример: `coins = [1, 3, 4]`, `amount = 6`. Жадно: `4 + 1 + 1 = 3` монеты, а оптимально `3 + 3 = 2` монеты. Значит нужен полный перебор вариантов — DP.

Определим `dp[i]` — минимальное количество монет, чтобы набрать сумму `i`. Переход: чтобы набрать сумму `i`, последняя добавленная монета — какая-то `coin` из `coins`, а до неё нужно было набрать `i - coin` минимальным числом монет. Перебираем все возможные "последние монеты":

`dp[i] = min(dp[i - coin] + 1)` по всем `coin` из `coins`, где `coin <= i`.

База: `dp[0] = 0` (сумму 0 набираем нулём монет).

### Решение

```cpp
#include "coin_change.h"  
  
#include <vector>  
#include <limits>  
#include <algorithm>  
#include <iostream>  
#include <format>  
  
namespace coin_change {  
  
int coin_change(const std::vector<int>& coins, const int amount) {  
    constexpr int INF{std::numeric_limits<int>::max() / 2};  
    std::vector<int> dp(amount + 1, INF);  
    dp[0] = 0;  
  
    for (int i{}; i <= amount; ++i) {  
        for (const int coin: coins) {  
            if (coin <= i && dp[i-coin] != INF) {  
                dp[i] = std::min(dp[i], dp[i-coin] + 1);  
            }        
        }    
    }  

    return dp[amount] == INF ? -1 : dp[amount];  
}  
  
void demo() {  
    constexpr int AMOUNT{11};  
    const std::vector<int> COINS{1, 2, 5};  
  
    std::cout << std::format("quantity: {}\n", coin_change(COINS, AMOUNT));  
}  
  
}
```

### Разбор

- `dp[i]` инициализируется как "недостижимо" (`INF`), кроме `dp[0] = 0` — базовый случай.
- Внешний цикл идёт по суммам от 1 до `amount` **по возрастанию** — это важно, т.к. `dp[i]` зависит только от **меньших** сумм (`i - coin < i`), которые уже точно посчитаны к этому моменту.
- Внутренний цикл перебирает все номиналы монет как кандидата на "последнюю добавленную монету" для суммы `i`.
- `dp[i - coin] != INF` — проверка, что подсумма `i - coin` вообще достижима; без неё пришлось бы либо ловить переполнение `INF + 1`, либо просто не портить корректный ответ мусорным значением (INF, делённый на 2, как раз даёт запас, чтобы `INF + 1` не переполнялся и не становился ложно "маленьким" числом).
- `INF = max/2` — стандартный трюк, чтобы `dp[i-coin] + 1` не переполнял `int` даже если `dp[i-coin]` уже "условно бесконечность".

### Пример

```
coins = [1, 2, 5], amount = 11

dp[0]=0
dp[1]: coin=1 -> dp[0]+1=1; coin=2,5 не подходят (>1)
       dp[1]=1
dp[2]: coin=1 -> dp[1]+1=2; coin=2 -> dp[0]+1=1; coin=5 не подходит
       dp[2]=min(2,1)=1
dp[3]: coin=1 -> dp[2]+1=2; coin=2 -> dp[1]+1=2; coin=5 не подходит
       dp[3]=2
dp[4]: coin=1 -> dp[3]+1=3; coin=2 -> dp[2]+1=2
       dp[4]=2
dp[5]: coin=1 -> dp[4]+1=3; coin=2 -> dp[3]+1=3; coin=5 -> dp[0]+1=1
       dp[5]=1
dp[6]: coin=1->dp[5]+1=2; coin=2->dp[4]+1=3; coin=5->dp[1]+1=2
       dp[6]=2
...
dp[11]: (пропущу промежуточные) итоговое dp[11]=3  (5+5+1)

Результат: 3
```

### Сложность

- Время: **O(amount · количество монет)** — для каждой суммы перебираем все номиналы.
- Память: **O(amount)** — под массив `dp`.

### Частые вариации

- **Coin Change II** — не минимальное число монет, а **количество способов** набрать сумму (порядок монет не важен) → внешний и внутренний циклы меняются местами (сначала по монетам, потом по суммам) — это классическое различие между "перестановками" и "сочетаниями" в DP по рюкзаку, частый вопрос "почему порядок циклов важен".
- **Perfect Squares** — тот же паттерн DP, но "монеты" — это квадраты чисел (`1, 4, 9, 16, ...`), минимальное число слагаемых для суммы `n`.
- **Combination Sum IV** — по сути Coin Change II, но с учётом порядка (перестановки считаются разными комбинациями) → циклы в исходном порядке (сумма снаружи, монеты внутри), как в решении выше.

### Частый доп. вопрос: "почему порядок циклов имеет значение?"

Если сумма `i` — внешний цикл, а монеты — внутренний (как в решении выше), то для каждой суммы перебираются все монеты **независимо от предыдущего выбора**, что корректно для задачи "минимальное число монет" (порядок использования монет не важен, важен только факт достижимости с минимальным количеством). Но если задача — "количество способов **набора без учёта порядка**" (Coin Change II), нужно менять местами циклы (монеты снаружи), чтобы не считать одну и ту же комбинацию монет в разном порядке несколько раз — это важный нюанс, который часто проверяют на собеседовании отдельным вопросом.

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
- [ ] Dynamic - Longest Common Subsequence
- [ ] Dynamic - Longest Increasing Subsequence
- [ ] Dynamic - Edit Distance
- [ ] Dynamic - House Robber
- [ ] Dynamic - Word Break
- [ ] Dynamic - 0/1 Knapsack
- [ ] hear/stack/queue - Min Stack
- [ ] hear/stack/queue - Kth Largest Element in an Array
- [ ] hear/stack/queue - Top K Frequent Elements
- [ ] hear/stack/queue - Sliding Window Maximum
- [ ] backtracking - Permutations
- [ ] backtracking - Subsets
- [ ] backtracking - N-Queens
- [ ] backtracking - Combination Sum


**Прочее (жадные, бинарный поиск, дизайн)** 41. Binary Search 42. Search in Rotated Sorted Array 43. Trapping Rain Water 44. Merge K Sorted Lists 45. Design a Trie (Prefix Tree)
