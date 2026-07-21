[[raw data/cpp/interview/_|<=]]

## Maximum Subarray (Kadane's algorithm)

**Условие:** дан массив целых чисел `nums` (может содержать отрицательные). Найти непрерывный подмассив с максимальной суммой и вернуть эту сумму. Подмассив должен быть непустым.

### Идея

Наивно — перебор всех подотрезков даёт O(n²) (или O(n³) без префиксных сумм). Алгоритм Кадане делает это за один проход: для каждой позиции `i` поддерживаем `curSum` — максимальную сумму подмассива, **заканчивающегося именно в `i`**. Переход: либо продолжаем предыдущий подмассив (`curSum + nums[i]`), либо начинаем новый с текущего элемента (`nums[i]`), — выбираем то, что больше. Ответ — максимум `curSum` по всем `i`.

Ключевая идея: если накопленная сумма `curSum` стала отрицательной, тащить её дальше невыгодно — лучше "сбросить" и начать новый подмассив с текущего элемента.

### Решение

```cpp
#include "max_subarray.h"  
  
#include <tuple>  
#include <vector>  
#include <iostream>  
#include <format>  
  
namespace max_subarray {  
  
std::tuple<int, int, int> find_max_subarray(const std::vector<int>& sequence) {  
    int cur_sum{sequence[0]};  
    int best_sum{sequence[0]};  
    int start{0};  
    int best_start{0};  
    int best_end{0};  
  
    for (int i{1}; i < static_cast<int>(sequence.size()); ++i) {  
        if (cur_sum < 0) {  
            cur_sum = sequence[i];  
            start = i;  
        } else {  
            cur_sum += sequence[i];  
        }  
        if (cur_sum > best_sum) {  
            best_sum = cur_sum;  
            best_start = start;  
            best_end = i;  
        }
	}  

    return {best_sum, best_start, best_end};  
}  
  
void demo() {  
    const std::vector<int> SEQUENCE{1, 2, 3, 4, -1, 5, 6, 7, 8, -10, -20, 9, 7};  
    auto [sum, begin, end] = find_max_subarray(SEQUENCE);  
    std::cout << std::format("[{}, {}] => {}\n",begin, end, sum);  
}  
  
}
```

### Разбор

- `curSum` — лучшая сумма подмассива, заканчивающегося в текущем элементе.
- `std::max(nums[i], curSum + nums[i])` — эквивалентно `if (curSum < 0) curSum = nums[i]; else curSum += nums[i];`, но короче.
- `bestSum` инициализирован `nums[0]`, а не 0 — это важно, потому что все числа могут быть отрицательными, а подмассив обязан быть непустым. Если бы стартовали с 0, при всех отрицательных числах ответ был бы неверно 0.
- Здесь `i` сделан `size_t` без каста, т.к. сравнивается напрямую с `nums.size()` без смешивания со знаковым — в отличие от прошлой задачи, где `i` использовался как возвращаемое значение типа `int`.

### Сложность

- Время: **O(n)** — один проход.
- Память: **O(1)**.
### Частые вариации

- **Maximum Product Subarray** — из-за отрицательных чисел нужно хранить и `maxProd`, и `minProd` на каждом шаге (перемножение двух отрицательных даёт положительное).
- **Maximum Sum Circular Subarray** — сравнить обычный Kadane с `totalSum - minSubarraySum` (случай, когда лучший подмассив "оборачивается" через конец массива).
- **Разделяй и властвуй** — альтернативное решение за O(n log n), часто спрашивают как доп. вопрос.

---
---
---

- [x] Array/String - Two Sum (2026.07.18)
- [x] Array/String - Best Time to Buy and Sell Stock (2026.07.18)
- [x] Array/String - Maximum Subarray (Kadane's algorithm) (2026.07.18)
- [ ] Array/String - Merge Intervals
- [ ] Array/String - Product of Array Except Self
- [ ] Array/String - Longest Substring Without Repeating Characters
- [ ] Array/String - Group Anagrams
- [ ] Array/String - Valid Parentheses
- [ ] Array/String - Container With Most Water
- [ ] Array/String - 3Sum
- [ ] LinkedList - Reverse Linked List
- [ ] LinkedList - Detect Cycle in Linked List (Floyd's)
- [ ] LinkedList - Merge Two Sorted Lists
- [ ] LinkedList - Remove Nth Node From End of List
- [ ] LinkedList - LRU Cache (список + hash map)
- [ ] Trees/Graphs - Maximum Depth of Binary Tree
- [ ] Trees/Graphs - Validate Binary Search Tree
- [ ] Trees/Graphs - Binary Tree Level Order Traversal
- [ ] Trees/Graphs - Lowest Common Ancestor of a BST/BT
- [ ] Trees/Graphs - Serialize and Deserialize Binary Tree
- [ ] Trees/Graphs - Number of Islands (BFS/DFS)
- [ ] Trees/Graphs - Clone Graph
- [ ] Trees/Graphs - Course Schedule (topological sort)
- [ ] Trees/Graphs - Word Ladder (BFS)
- [ ] Dynamic - Climbing Stairs
- [ ] Dynamic - Coin Change
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
