[[raw data/cpp/interview/_|<=]]

## Edit Distance (Levenshtein Distance)

**Условие:** даны две строки `word1` и `word2`. Найти минимальное количество операций (вставка, удаление, замена одного символа), чтобы превратить `word1` в `word2`.

### Идея

2D DP, похожая по структуре на Longest Common Subsequence. Определим `dp[i][j]` — минимальное число операций, чтобы превратить первые `i` символов `word1` в первые `j` символов `word2`. Переход зависит от того, совпадают ли последние символы:

- Если `word1[i-1] == word2[j-1]` — последние символы уже совпадают, операция не нужна: `dp[i][j] = dp[i-1][j-1]`.
- Если не совпадают — рассматриваем 3 возможные операции и берём минимум:
    - **замена** последнего символа `word1[i-1]` на `word2[j-1]`: `dp[i-1][j-1] + 1`;
    - **удаление** последнего символа `word1[i-1]`: `dp[i-1][j] + 1`;
    - **вставка** символа `word2[j-1]` в конец `word1`: `dp[i][j-1] + 1`.

База: `dp[i][0] = i` (удалить все `i` символов первой строки, чтобы получить пустую), `dp[0][j] = j` (вставить все `j` символов, чтобы из пустой строки получить вторую).

### Решение

```cpp
#include "edit_distance.h"  
  
#include <string>  
#include <vector>  
#include <algorithm>  
#include <iostream>  
#include <format>  
  
namespace edit_distance {  
  
int min_distance(const std::string& word1, const std::string& word2) {  
    const int N{static_cast<int>(word1.size())};  
    const int M{static_cast<int>(word2.size())};  
  
    std::vector<std::vector<int>> dp(N+1, std::vector<int>(M+1, 0));  
    for (int i{}; i <= N; ++i) dp[i][0] = i;  
    for (int j{}; j <= M; ++j) dp[0][j] = j;  
  
    const auto print = [&dp](const int _i, const int _j) {  
        std::cout << std::format("{} {}\n", _i, _j);        
        for (const auto& vec: dp) {  
            for (const auto& item: vec) {  
                std::cout << std::format("{} ", item);            
            }            
            std::cout << '\n';        
        }        
        std::cout << "\n\n";    
    };  
  
    for (int i{1}; i <= N; ++i) {  
        for (int j{1}; j <= M; ++j) {  
            if (word1[i-1] == word2[j-1]) {  
                dp[i][j] = dp[i-1][j-1];  
            } else {  
                dp[i][j] = 1 + std::min({  
                    dp[i-1][j-1], // замена  
                    dp[i-1][j], // удаление  
                    dp[i][j-1] // вставка  
                });  
            }            
            // print(i, j);  
        }  
    }  
    return dp[N][M];  
}  
  
int min_distance_opt(const std::string& word1, const std::string& word2) {  
    const int N{static_cast<int>(word1.size())};  
    const int M{static_cast<int>(word2.size())};  
  
    std::vector<int> prev(M+1);  
    std::vector<int> current(M+1);  
    for (int j{}; j <= M; ++j) prev[j] = j;  
  
    for (int i{1}; i <= N; ++i) {  
        current[0] = i;  
        for (int j{1}; j <= M; ++j) {  
            if (word1[i - 1] == word2[j - 1]) {  
                current[j] = prev[j-1];  
            } else {  
                current[j] = 1 + std::min({  
                    current[j-1],  
                    current[j],  
                    current[j-1]});  
            }        
        }        
        prev = current;  
    }  
    return prev[M];  
}  
  
void demo() {  
    const std::string word1{"horse"};  
    const std::string word2{"ros"};  
    std::cout << std::format("min_distance {}\n", min_distance(word1, word2));  
    std::cout << std::format("min_distance_opt {}\n", min_distance_opt(word1, word2));  
}  
  
}
```

### Разбор

- Базовые случаи `dp[i][0] = i` и `dp[0][j] = j` отражают "тривиальные" преобразования: превратить строку из `i` символов в пустую требует `i` удалений; превратить пустую строку в строку из `j` символов требует `j` вставок.
- При совпадении символов операция не тратится — просто наследуем результат подзадачи без последних символов (`dp[i-1][j-1]`).
- При несовпадении — три перехода соответствуют трём разрешённым операциям, применённым к **последнему символу**:
    - `dp[i-1][j-1] + 1` — заменили `word1[i-1]` на `word2[j-1]`, дальше решаем подзадачу без обоих последних символов;
    - `dp[i-1][j] + 1` — удалили `word1[i-1]` из первой строки, она стала на 1 короче, а вторая строка не изменилась;
    - `dp[i][j-1] + 1` — вставили в конец `word1` символ `word2[j-1]`, теперь этот символ совпал, а нужно решить подзадачу для оставшейся части `word2` без последнего символа.
- `std::min({...})` — список инициализации, компактнее, чем вложенные `std::min(std::min(a,b),c)`.

### Пример

```
word1 = "horse", word2 = "ros"

      ""  r  o  s
  ""   0  1  2  3
  h    1  1  2  3
  o    2  2  1  2
  r    3  2  2  2
  s    4  3  3  2
  e    5  4  4  3

dp[1][1]: 'h'!='r' -> 1+min(dp[0][0],dp[0][1],dp[1][0])=1+min(0,1,1)=1
dp[2][2]: 'o'=='o' -> dp[1][1]=1
dp[3][1]: 'r'=='r' -> dp[2][0]=2
dp[5][3]: 'e'!='s' -> 1+min(dp[4][2],dp[4][3],dp[5][2])=1+min(3,2,3)=3

Результат: dp[5][3] = 3
("horse" -> "rorse" (замена h->r) -> "rose" (удаление r) -> "ros" (удаление e))
```

### Сложность

- Время: **O(n · m)** — заполнение всей таблицы.
- Память: **O(n · m)**.

### Оптимизация памяти до O(min(n, m))

Аналогично LCS — каждая строка таблицы зависит только от предыдущей строки и текущей позиции слева:

Время: O(n·m) без изменений, память: **O(m)** вместо O(n·m).

### Частые вариации

- **One Edit Distance** — проверка, отличаются ли строки ровно на одну операцию — можно решить за O(n) без полной DP, сравнением посимвольно с ранним выходом.
- **Delete Operation for Two Strings** — только удаления (без вставок и замен) — сводится к LCS: ответ `= n + m - 2·LCS(word1, word2)`.
- **Longest Common Subsequence** — уже разобранная задача, структурно очень похожая DP-таблица, но с другим смыслом переходов.
- **Wildcard Matching / Regular Expression Matching** — тоже 2D DP по двум строкам, но с более сложными правилами перехода из-за спецсимволов `*`, `?`.

### Частый доп. вопрос: "почему замена — это диагональный переход, а не отдельный третий случай?"

Обратите внимание, что при совпадении символов используется тот же диагональный переход `dp[i-1][j-1]`, что и при замене — разница лишь в том, добавляется ли `+1`. Это подчёркивает единообразие структуры: замена — это как бы "совпадение с доплатой", если символы разные.
