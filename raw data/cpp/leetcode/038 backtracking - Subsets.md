[[raw data/cpp/interview/_|<=]]

## Subsets

**Условие:** дан массив `nums` из уникальных чисел. Вернуть все возможные подмножества (включая пустое множество и само `nums`) — то есть весь **степенной набор (power set)**.

### Идея

Для каждого элемента массива есть ровно два состояния: он либо **включён**, либо **не включён** в текущее подмножество. Это классический backtracking с бинарным выбором на каждом шаге — дерево рекурсии глубины `n`, где на каждом уровне два варианта, итого `2ⁿ` листьев (подмножеств).

Альтернативный взгляд (часто более нагляден для реализации): на каждом шаге рекурсии текущий `path` **уже является** валидным подмножеством — сохраняем его сразу при входе в рекурсию, а затем перебираем, какой из **оставшихся** элементов добавить следующим.

```cpp
#include "subsets.h"  
  
#include <string>  
#include <vector>  
#include <iostream>  
#include <format>  
  
namespace subsets {  
// index start solution  
void backtrace(const std::vector<int>& nums,  
                const int start,  
                std::vector<int>& path,  
                std::vector<std::vector<int>>& result) {  
    // текущий path — уже готовое подмножество, сохраняем сразу  
    result.push_back(path);  
  
    for (int i{start}; i < static_cast<int>(nums.size()); ++i) {  
        path.push_back(nums[i]);  
        // следующий элемент выбираем только из "правее i"  
        backtrace(nums, i + 1, path, result);  
        path.pop_back();  
    }}  
std::vector<std::vector<int>> subsets(const std::vector<int>& nums) {  
    std::vector<std::vector<int>> result;  
    std::vector<int> path;  
    backtrace(nums, 0, path, result);  
  
    return result;  
}  
  
// bitmask solution  
std::vector<std::vector<int>> subsets_bitmask(const std::vector<int>& nums) {  
    const int N{static_cast<int>(nums.size())};  
    std::vector<std::vector<int>> result;  
  
    for (int mask{}; mask < (1 << N); ++mask) {  
        std::vector<int> subset;  
        for (int j{}; j < N; ++j) {  
            if (mask & (1 << j)) {  
                subset.push_back(nums[j]);  
            }        
        }        
	    result.push_back(std::move(subset));  
    }  
    return result;  
}  
  
// iterative solution  
std::vector<std::vector<int>> subsets_iterative(const std::vector<int>& nums) {  
    // начинаем с пустого подмножества  
    std::vector<std::vector<int>> result{{}};  
  
    for (const auto& num : nums) {  
        const int SIZE{static_cast<int>(result.size())};  
        for (int i{}; i < SIZE; ++i) {  
            std::vector<int> new_subset = result[i];  
            new_subset.push_back(num);  
            result.push_back(std::move(new_subset));  
        }    
    }  
    return result;  
}  
  
static void print(const std::vector<std::vector<int>>& nums, const std::string& prefix) {  
    std::cout << std::format("{}: ", prefix);  
    for (const auto& vec : nums) {  
        for (const auto& num : vec) {  
            std::cout << std::format("{} ", num);  
        }        
        std::cout << '\n';  
    }    
    std::cout << "\n\n";  
}  
  
void demo() {  
    const std::vector<int> NUMS{1, 2, 3};  
  
    const auto result0 = subsets(NUMS);  
    print(result0, "START");  
  
    const auto result1 = subsets_bitmask(NUMS);  
    print(result1, "MASK");  
  
    const auto result2 = subsets_iterative(NUMS);  
    print(result2, "ITERATIVE");  
}  
}
```

### Решение (через "старт индекса", наиболее распространённый способ)

### Разбор

- `result.push_back(path)` вызывается **на каждом входе в рекурсию**, а не только в "базовом случае" — потому что каждый промежуточный `path` (в том числе пустой в самом начале) уже является полноценным подмножеством, а не только листья дерева рекурсии.
- Параметр `start` гарантирует, что на каждом уровне рекурсии рассматриваются только элементы **правее** уже выбранных — это исключает повторный подсчёт одного и того же подмножества в разном порядке (например, `[1,2]` и `[2,1]` не должны быть посчитаны как разные подмножества, поскольку порядок в подмножестве не важен).
- Цикл `for (i = start; ...)` перебирает, какой из оставшихся элементов добавить следующим в `path`; после рекурсивного вызова — откат (`pop_back`), чтобы освободить `path` для следующей итерации цикла на этом уровне.

### Пример

```
nums = [1,2,3]

backtrack(start=0, path=[])
  result=[[]] (сохранили пустое подмножество)
  i=0: path=[1]
    backtrack(start=1, path=[1])
      result=[[],[1]]
      i=1: path=[1,2]
        backtrack(start=2, path=[1,2])
          result=[...,[1,2]]
          i=2: path=[1,2,3]
            backtrack(start=3, path=[1,2,3])
              result=[...,[1,2,3]]
              (цикл: start=3 >= nums.size()=3 -> не выполняется)
            откат: path=[1,2]
        откат: path=[1]
      i=2: path=[1,3]
        backtrack(start=3, path=[1,3]) -> result=[...,[1,3]]
        откат: path=[1]
    откат: path=[]
  i=1: path=[2]
    backtrack(start=2, path=[2]) -> result=[...,[2]]
      i=2: path=[2,3] -> result=[...,[2,3]]
    откат: path=[]
  i=2: path=[3]
    backtrack(start=3, path=[3]) -> result=[...,[3]]
    откат: path=[]

Итог: [[], [1], [1,2], [1,2,3], [1,3], [2], [2,3], [3]]
```

### Сложность

- Время: **O(2ⁿ · n)** — существует `2ⁿ` подмножеств, каждое требует до O(n) на копирование в `result`.
- Память: **O(n)** доп. памяти на стек рекурсии и `path` (не считая памяти под результат, неизбежно O(2ⁿ · n)).

### Альтернатива: битовая маска (итеративно, без рекурсии)

Каждое подмножество можно закодировать `n`-битным числом от `0` до `2ⁿ - 1`, где бит `j` показывает, входит ли `nums[j]` в это подмножество.

**Разбор:** `mask` пробегает все числа от `0` до `2ⁿ-1` — каждое такое число однозначно кодирует одно подмножество через свою битовую запись. `mask & (1 << j)` проверяет, установлен ли `j`-й бит — если да, `nums[j]` входит в это подмножество. Concептуально проще для понимания через "бинарный выбор", но требует явного знания размера `n` заранее (в отличие от backtracking, который естественно обобщается, например, на Subsets II с дубликатами).

Время: **O(2ⁿ · n)**, память: аналогично.

### Альтернатива: итеративное построение "удвоением" (без рекурсии и битовых масок)

Разбор: для каждого нового числа `num` берём **все уже существующие** подмножества и создаём их копии с добавленным `num` — это удваивает количество подмножеств на каждом шаге (`1 → 2 → 4 → 8 → ...`), что и даёт итоговые `2ⁿ`.

### Частые вариации

- **Subsets II** — входной массив содержит **дубликаты**, нужны только уникальные подмножества → сортировка + пропуск повторов на одном уровне рекурсии (`if (i > start && nums[i] == nums[i-1]) continue;`), похожий приём на Permutations II.
- **Combination Sum** — подмножества с фиксированной суммой элементов, при этом один элемент можно использовать многократно → цикл продолжает с того же индекса `i` (не `i+1`) при рекурсивном вызове.
- **Partition to K Equal Sum Subsets** — более сложная вариация с разбиением на несколько групп равной суммы — backtracking с доп. проверками для отсечения (pruning) заведомо бесперспективных веток.
- **Generate Parentheses** — тоже backtracking с "выбором на каждом шаге", но выбор ограничен правилами валидности скобочной последовательности, а не просто "включить/не включить".
