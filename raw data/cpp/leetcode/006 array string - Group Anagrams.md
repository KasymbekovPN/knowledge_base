[[raw data/cpp/interview/_|<=]]

## Group Anagrams

**Условие:** дан массив строк `strs`. Сгруппировать анаграммы вместе — строки, состоящие из одинакового набора символов в разном порядке. Вернуть список групп (порядок групп и порядок внутри групп не важен).

### Идея

Две строки — анаграммы тогда и только тогда, когда у них одинаковый "канонический вид". Самый простой канонический вид — отсортированная строка: `"eat"` и `"tea"` после сортировки символов дают одну и ту же строку `"aet"`. Используем этот отсортированный вариант как ключ хеш-таблицы: `ключ → список исходных строк с таким ключом`.

### Решение

```cpp
#include "group_anagram.h"  
  
#include <iostream>  
#include <format>  
#include <algorithm>  
#include <ranges>  
#include <unordered_map>  
  
namespace group_anagram {  
  
std::vector<std::vector<std::string>> group_anagrams(std::vector<std::string> &input) {  
    std::unordered_map<std::string, std::vector<std::string>> groups;  
    groups.reserve(input.size());  
  
    for (auto& s: input) {  
        std::string key{s};  
        std::ranges::sort(key.begin(), key.end());  
        groups[key].push_back(std::move(s));  
    }  
    std::vector<std::vector<std::string>> result;  
    result.reserve(groups.size());  
    for (auto &group: groups | std::views::values) {  
        result.push_back(std::move(group));  
    }

    return result;  
}  
  
void demo() {  
    std::vector<std::string> LINES{"eat", "tea", "tan", "ate", "nat", "bat"};  
    for (const auto& vectors = group_anagrams(LINES); auto& vector : vectors) {  
        std::string delimiter;  
        std::cout << "{";  
        for (auto& item : vector) {  
            std::cout << std::format("{}{}", delimiter, item);  
            delimiter = ", ";  
        }
		std::cout << "}\n";  
    }
}  
  
}
```

### Разбор

- Для каждой строки `s` строим `key` — её отсортированную копию. Все анаграммы дают идентичный `key`.
- `groups[key].push_back(std::move(s))` — добавляем **оригинальную** (не отсортированную) строку в группу по этому ключу; `std::move` избегает лишнего копирования, так как `s` больше не нужна в исходном виде.
- В конце просто выгружаем все значения хеш-таблицы в результирующий вектор.
- Сортировка каждой строки стоит `O(k log k)`, где `k` — её длина; делаем это для всех `n` строк.

### Пример

```
strs = ["eat", "tea", "tan", "ate", "nat", "bat"]

"eat" -> key "aet" -> groups["aet"] = ["eat"]
"tea" -> key "aet" -> groups["aet"] = ["eat", "tea"]
"tan" -> key "ant" -> groups["ant"] = ["tan"]
"ate" -> key "aet" -> groups["aet"] = ["eat", "tea", "ate"]
"nat" -> key "ant" -> groups["ant"] = ["tan", "nat"]
"bat" -> key "abt" -> groups["abt"] = ["bat"]

Результат: [["eat","tea","ate"], ["tan","nat"], ["bat"]]
```

### Сложность

- Время: **O(n · k log k)**, где `n` — число строк, `k` — максимальная длина строки (сортировка каждой строки).
- Память: **O(n · k)** — под хеш-таблицу и результат.

### Оптимизация ключа без сортировки

Если известно, что символы только из `'a'`..`'z'`, можно построить ключ за `O(k)` вместо `O(k log k)` — посчитать частоты символов (`std::array<int, 26>`) и превратить в строку-сигнатуру (например, `"#1#0#0...#2"` — количество каждой буквы). Тогда общая сложность падает до **O(n · k)**.

```cpp
std::string countKey(const std::string& s) {
    std::array<int, 26> count{};
    for (char c : s) ++count[c - 'a'];

    std::string key;
    for (int c : count) {
        key += '#';
        key += std::to_string(c);
    }
    return key;
}
```

Это классический доп. вопрос на собеседовании: "а можно ли быстрее сортировки?" — да, за счёт подсчёта частот вместо сортировки.
