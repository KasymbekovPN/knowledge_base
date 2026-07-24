[[raw data/cpp/interview/_|<=]]

## Permutations

**Условие:** дан массив `nums` из уникальных чисел. Вернуть все возможные перестановки (permutations) этого массива.

### Идея

Классическая задача на **backtracking**. Строим перестановку поэлементно: на каждом шаге выбираем один из ещё не использованных элементов, добавляем его в текущий "путь" (частично построенную перестановку), рекурсивно продолжаем для оставшихся позиций, а после возврата из рекурсии **откатываем** выбор (убираем элемент из пути и помечаем его снова доступным) — это и есть "backtracking", позволяющий перебрать все варианты, переиспользуя один и тот же буфер `path`.

### Решение

```cpp
#include "backtrack_permutations.h"  
  
#include <vector>  
#include <string>  
#include <iostream>  
#include <format>  
  
namespace backtrack_permutations {  
  
void backtrack(std::vector<int>& nums,  
                std::vector<int>& path,  
                std::vector<bool>& used,  
                std::vector<std::vector<int>>& result) {  
    if (path.size() == nums.size()) {  
        // путь полностью построен -> сохраняем копию  
        result.push_back(path);  
        return;  
    }  
    for (size_t i{0}; i < nums.size(); ++i) {  
        if (used[i]) continue;  
  
        used[i] = true;  
        path.push_back(nums[i]);  
  
        backtrack(nums, path, used, result);  
  
        // откат: убираем последний выбранный элемент  
        path.pop_back();  
        // откат: помечаем элемент снова доступным  
        used[i] = false;  
    }
}  
  
std::vector<std::vector<int>> permute(std::vector<int>& nums) {  
    std::vector<std::vector<int>> result;  
    std::vector<int> path;  
    std::vector<bool> used(nums.size(), false);  
  
    backtrack(nums, path, used, result);  
    return result;  
}  
  
void backtrack_swap(std::vector<int>& nums, const int start, std::vector<std::vector<int>>& result) {  
    if (start == static_cast<int>(nums.size())) {  
        result.push_back(nums);  
        return;  
    }  
    for (int i{start}; i < static_cast<int>(nums.size()); ++i) {  
        std::swap(nums[start], nums[i]);  
        backtrack_swap(nums, start + 1, result);  
        // откат обмена  
        std::swap(nums[start], nums[i]);  
    }
}  
  
void demo() {  
    std::vector<int> nums0{1, 2, 3};  
    const auto result0 = permute(nums0);  
  
    std::vector<int> nums1{1, 2, 3};  
    std::vector<std::vector<int>> result1;  
    backtrack_swap(nums1, 0, result1);  
  
    const auto print = [](const std::vector<std::vector<int>>& nums, const std::string &label) {  
        std::cout << std::format("{} \n", label);  
        for (const auto& vec: nums) {  
            for (const auto& i: vec) {  
                std::cout << std::format("{} ", i);  
            }            std::cout << '\n';  
        }        std::cout << "\n\n";  
    };  
    print(result0, "R0");  
    print(result1, "R1");  
}  
  
}
```

### Разбор

- `path` — буфер текущей строящейся перестановки; `used[i]` — отмечает, задействован ли элемент `nums[i]` в этом пути прямо сейчас.
- База рекурсии: `path.size() == nums.size()` — путь заполнен полностью, это готовая перестановка. `result.push_back(path)` делает **копию** текущего `path` (не ссылку — иначе все сохранённые результаты указывали бы на один и тот же изменяющийся буфер).
- Цикл `for` на каждом уровне рекурсии перебирает **все** ещё не использованные элементы как кандидата на следующую позицию — отсюда `n` вариантов на первом уровне, `n-1` на втором и т.д. — суммарно `n!` листьев дерева рекурсии.
- Ключевая часть backtracking — три шага "выбор → рекурсия → откат": `push_back` + `used[i]=true` (выбрали элемент), рекурсивный вызов (строим дальше), `pop_back` + `used[i]=false` (отменили выбор, чтобы освободить этот элемент для других веток перебора на этом уровне).
- Без отката (`pop_back`/`used[i]=false`) `path` и `used` продолжали бы накапливать состояние от предыдущих веток, и корректный перебор всех вариантов был бы невозможен.

### Пример

```
nums = [1,2,3]

backtrack([], used=[F,F,F])
  i=0: used[0]=T, path=[1]
    backtrack([1], used=[T,F,F])
      i=1: used[1]=T, path=[1,2]
        backtrack([1,2], used=[T,T,F])
          i=2: used[2]=T, path=[1,2,3]
            path.size()==3 -> result=[[1,2,3]]
          откат: path=[1,2], used[2]=F
      откат: path=[1], used[1]=F
      i=2: used[2]=T, path=[1,3]
        backtrack([1,3], ...)
          i=1: path=[1,3,2] -> result=[[1,2,3],[1,3,2]]
        откат
      откат: path=[1], used[2]=F
    откат: path=[], used[0]=F
  i=1: used[1]=T, path=[2]
    ... аналогично даёт [2,1,3] и [2,3,1]
  i=2: used[2]=T, path=[3]
    ... аналогично даёт [3,1,2] и [3,2,1]

Результат: [[1,2,3],[1,3,2],[2,1,3],[2,3,1],[3,1,2],[3,2,1]]
```

### Сложность

- Время: **O(n! · n)** — существует `n!` перестановок, и построение/копирование каждой занимает O(n).
- Память: **O(n)** доп. памяти на стек рекурсии и буферы `path`/`used` (не считая памяти под сам результат, который неизбежно O(n! · n)).

### Альтернатива: перестановка на месте через swap (без `used`)

Часто показывают и другой классический вариант backtracking для перестановок — без вспомогательного массива `used`, а через обмен элементов местами прямо в исходном массиве:

Разбор: `nums[0..start-1]` — уже "зафиксированная" часть перестановки, `nums[start..end]` — ещё не определённая часть. На позицию `start` пробуем поставить по очереди каждый из оставшихся элементов (обмен местами), рекурсивно фиксируем следующую позицию, затем обмен откатывается обратно перед следующей итерацией цикла. Экономит память на `used` и `path` (перестановка стоится прямо в `nums`), но чуть менее нагляден.

### Частые вариации

- **Permutations II** — входной массив содержит **дубликаты**, нужны только уникальные перестановки → добавляется сортировка + пропуск дубликатов на одном уровне рекурсии (`if (i > 0 && nums[i] == nums[i-1] && !used[i-1]) continue;`).
- **Next Permutation** — не перебор всех перестановок, а нахождение следующей по лексикографическому порядку — решается без рекурсии, через анализ суффикса массива.
- **Combinations** — похожий backtracking, но порядок элементов не важен (сочетания, а не перестановки) → цикл начинается с индекса `start` вместо перебора всех неиспользованных, что естественно исключает дублирующиеся по порядку варианты.
- **Letter Case Permutation** — backtracking с бинарным выбором на каждом шаге (менять регистр буквы или нет) вместо выбора из `n` вариантов.
