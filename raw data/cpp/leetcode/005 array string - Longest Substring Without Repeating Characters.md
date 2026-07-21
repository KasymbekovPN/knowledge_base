[[raw data/cpp/interview/_|<=]]

## Longest Substring Without Repeating Characters

**Условие:** дана строка `s`. Найти длину самой длинной подстроки без повторяющихся символов.

### Идея

Наивно — перебор всех подстрок с проверкой уникальности символов: O(n³) (или O(n²) с проверкой за O(n)). Оптимально — **скользящее окно (sliding window)** с двумя указателями `left` и `right`. Расширяем окно вправо; если символ `s[right]` уже есть внутри текущего окна — сдвигаем `left` до тех пор, пока дубликат не выйдет из окна. Для быстрой проверки "символ уже в окне" и мгновенного узнавания, куда сдвинуть `left`, храним последнюю позицию каждого символа в хеш-таблице.

### Решение

```cpp
#include "len_of_longest_substring.h"  
  
#include <iostream>  
#include <format>  
#include <unordered_map>  
#include <algorithm>  
  
namespace len_of_longest_substring {  
  
int get_longest_substring(const std::string &line) {  
    std::unordered_map<char, int> last_seen;  
    int best{};  
    int left{};  
  
    for (int right{}; right < static_cast<int>(line.length()); ++right) {  
        char c{line[right]};  
        if (auto it = last_seen.find(c); it != last_seen.end() && it->second >= left) {  
            left = it->second + 1; // сдвигаем левую границу за дубликат  
        }  
  
        last_seen[c] = right;  
        best = std::max(best, right - left + 1);  
    }  
    return best;  
}  
  
void demo() {  
    const std::string line{"0123011234566789"};  
    std::cout << std::format("best: {}\n", get_longest_substring(line));  
}  
  
}
```

### Разбор

- `lastSeen[c]` хранит индекс последнего вхождения символа `c`.
- Условие `it->second >= left` критично: без него можно ошибочно сдвинуть `left` назад по устаревшей записи о символе, который встречался **до** текущего окна (уже вышел из него раньше). Проверка гарантирует, что мы двигаем `left` только если дубликат реально находится внутри текущего окна.
- `left = it->second + 1` — прыгаем сразу за позицию дубликата, а не сдвигаем `left` по одному символу — за счёт этого весь алгоритм линеен, а не O(n²).
- На каждом шаге текущая длина окна `right - left + 1` сравнивается с лучшим результатом.
- Каждый указатель (`left`, `right`) двигается только вперёд и суммарно не более `n` раз каждый — отсюда линейная сложность, несмотря на вложенную логику.

### Пример

```
s = "abcabcbb"

right=0 'a': lastSeen={}, left=0, best=1, lastSeen={a:0}
right=1 'b': best=2, lastSeen={a:0,b:1}
right=2 'c': best=3, lastSeen={a:0,b:1,c:2}
right=3 'a': 'a' видели на 0, 0>=left(0) -> left=1; best=max(3,3)=3; lastSeen[a]=3
right=4 'b': 'b' видели на 1, 1>=left(1) -> left=2; best=3; lastSeen[b]=4
right=5 'c': 'c' видели на 2, 2>=left(2) -> left=3; best=3; lastSeen[c]=5
right=6 'b': 'b' видели на 4, 4>=left(3) -> left=5; best=3; lastSeen[b]=6
right=7 'b': 'b' видели на 6, 6>=left(5) -> left=7; best=3; lastSeen[b]=7

Ответ: 3  ("abc")
```

### Сложность

- Время: **O(n)** — оба указателя суммарно проходят по строке линейное число раз.
- Память: **O(min(n, alphabet size))** — под хеш-таблицу (например, O(1)=128 для ASCII, если использовать массив вместо `unordered_map`).

### Оптимизация

Если алфавит известен и мал (ASCII/lowercase), `std::array<int, 128>` вместо `unordered_map` даёт лучшую константу по времени и убирает overhead хеширования.

### Частые вариации

- **Longest Substring with At Most K Distinct Characters** — то же скользящее окно, но условие сжатия — количество уникальных символов в окне превышает `k`.
- **Longest Repeating Character Replacement** — окно + подсчёт частот, условие: `длина окна - count(most frequent char) <= k`.
- **Minimum Window Substring** — окно должно **содержать** все символы из другой строки (обратная задача — минимизация, а не максимизация окна).
