[[raw data/cpp/interview/_|<=]]

## Longest Common Subsequence

**Условие:** даны две строки `text1` и `text2`. Найти длину наибольшей общей подпоследовательности — последовательности символов, которая встречается в обеих строках в том же относительном порядке, но не обязательно подряд (в отличие от подстроки).

### Идея

Классическая 2D DP. Определим `dp[i][j]` — длина LCS префиксов `text1[0..i-1]` и `text2[0..j-1]` (первые `i` символов первой строки и первые `j` символов второй). Переход:

- Если последние символы совпадают (`text1[i-1] == text2[j-1]`) — этот символ точно входит в LCS, и `dp[i][j] = dp[i-1][j-1] + 1` (LCS префиксов без последнего символа, плюс 1 за общий символ).
- Если не совпадают — символ не может одновременно входить в LCS из обеих строк, значит берём лучший результат, отбросив последний символ **одной** из строк: `dp[i][j] = max(dp[i-1][j], dp[i][j-1])`.

База: `dp[0][j] = dp[i][0] = 0` — LCS с пустой строкой всегда равен 0.

### Решение

```cpp
#include "longest_common_subsequence.h"  
  
#include <iostream>  
#include <format>  
#include <string>  
#include <vector>  
#include <algorithm>  
  
namespace longest_common_subsequence {  
  
int calc_longest_common_subsequence(const std::string& text0, const std::string& text1) {  
    std::cout << "calc_longest_common_subsequence\n";  
    const int N{static_cast<int>(text0.size())};  
    const int M{static_cast<int>(text1.size())};  
  
    std::vector<std::vector<int>> dp(N + 1, std::vector<int>(M + 1, 0));  
    auto print = [&dp, &text0, &text1](const int i, const int j) {  
        std::cout << std::format("{}\n{}^\n{}\n{}^\n",            
	        text0,            
	        std::string(i, ' '),            
	        text1,            
	        std::string(j, ' '));
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
            if (text0[i-1] == text1[j-1]) {  
                dp[i][j] = dp[i-1][j-1] + 1;  
            } else {  
                dp[i][j] = std::max(dp[i-1][j], dp[i-1][j-1]);  
            }            
            // print(i, j + 1);  
        }  
    }  
    return dp[N][M];  
}  
  
int calc_longest_common_subsequence_opt(const std::string& text0, const std::string& text1) {  
    std::cout << "calc_longest_common_subsequence_opt\n";  
    const int N{static_cast<int>(text0.size())};  
    const int M{static_cast<int>(text1.size())};  
  
    std::vector<int> prev(M+1, 0);  
    std::vector<int> current(M+1, 0);  
    const auto print = [&prev](const int i, const int j) {  
        std::cout << std::format("{} {} [", i, j);        
        for (const auto& item: prev) std::cout << std::format("{} ", item);  
        std::cout << "]\n";    
    };  
  
    for (int i{1}; i <= N; ++i) {  
        for (int j{1}; j <= M; ++j) {  
            if (text0[i-1] == text1[j-1]) {  
                current[j] = prev[j-1] + 1;  
            } else {  
                current[j] = std::max(prev[j], current[j-1]);  
            }            
            // print (i, j);  
        }  
  
        // текущая строка становится "предыдущей" для следующей итерации  
        prev = current;  
    }  
    return prev[M];  
}  
  
std::string reconstruct_lcs(const std::string& text0, const std::string& text1, const std::vector<std::vector<int>>& dp) {  
    std::cout << "reconstruct_lcs\n";  
    std::string result;  
    int i{static_cast<int>(text0.size())};  
    int j{static_cast<int>(text1.size())};  
  
    auto print = [&dp, &text0, &text1, &i, &j]() {  
        std::cout << std::format("{}\n{}^\n{}\n{}^\n",            
	        text0,            
	        std::string(i, ' '),            
	        text1,            
	        std::string(j, ' '));        
	    for (const auto& vec: dp) {  
            for (const auto& item: vec) {  
                std::cout << std::format("{} ", item);            
            }            
            std::cout << '\n';        
        }  
        std::cout << "\n\n";    
    };  
  
    while (i > 0 && j > 0) {  
        if (text0[i-1] == text1[j-1]) {  
            result += text0[i-1];  
            --i; --j;  
        } else if (dp[i-1][j] >= dp[i][j-1]) {  
            --i;  
        } else {  
            --j;  
        }        
        // print();  
    }  
    std::ranges::reverse(result);  
  
    return  result;  
}  
  
void demo() {  
    const std::string text0{"abcde"};  
    const std::string text1{"ace"};  
  
    std::cout << calc_longest_common_subsequence(text0, text1) << '\n';  
    std::cout << calc_longest_common_subsequence_opt(text0, text1) << '\n';  
  
    std::vector<std::vector<int>> dp = {  
        {0, 0, 0, 0},  
        {0, 1, 0, 0},  
        {0, 1, 1, 0},  
        {0, 1, 2, 1},  
        {0, 1, 2, 2},  
        {0, 1, 2, 3}  
    };  
    std::cout << reconstruct_lcs(text0, text1, dp) << '\n';  
}  
  
}
```

### Разбор

- `dp[i][j]` индексируется от 1, а сами строки — от 0, поэтому символы сравниваются как `text1[i-1]` и `text2[j-1]` (сдвиг на единицу — стандартная практика в строковых DP, чтобы строка `dp[0][*]`/`dp[*][0]` естественно представляла "пустой префикс").
- Совпадение символов — символ гарантированно можно включить в LCS (доказывается тем, что если существует LCS без этого совпадения, всегда можно построить LCS не хуже, включив его) — берём диагональный переход `dp[i-1][j-1] + 1`.
- Несовпадение — пробуем оба варианта "отбросить последний символ одной из строк" и берём лучший — раз символы разные, они не могут оба одновременно входить в LCS этих суффиксов/префиксов.
- Таблица заполняется по возрастанию `i` и `j`, каждая ячейка зависит только от уже вычисленных соседей сверху, слева и по диагонали.

### Пример

```
text1 = "abcde", text2 = "ace"

      ""  a  c  e
  ""   0  0  0  0
  a    0  1  1  1
  b    0  1  1  1
  c    0  1  2  2
  d    0  1  2  2
  e    0  1  2  3

dp[1][1]: 'a'=='a' -> dp[0][0]+1=1
dp[1][2]: 'a'!='c' -> max(dp[0][2],dp[1][1])=max(0,1)=1
dp[3][2]: 'c'=='c' -> dp[2][1]+1=1+1=2
dp[5][3]: 'e'=='e' -> dp[4][2]+1=2+1=3

Результат: dp[5][3] = 3  (LCS = "ace")
```

### Сложность

- Время: **O(n · m)** — заполнение всей таблицы.
- Память: **O(n · m)** — сама таблица DP.

### Оптимизация памяти до O(min(n, m))

Каждая строка `dp[i][*]` зависит только от предыдущей строки `dp[i-1][*]` (и текущей `dp[i][j-1]`) — весь двумерный массив не нужен, достаточно двух строк (или даже одной, при аккуратной работе с временными переменными).

Время: O(n·m) без изменений, память: **O(m)** вместо O(n·m).

### Восстановление самой подпоследовательности (частый доп. вопрос)

Если нужно вывести саму строку LCS, а не только длину — проходим по заполненной таблице `dp` с конца, восстанавливая путь:

Требует полную 2D-таблицу (оптимизация до O(min(n,m)) памяти здесь не подходит, т.к. нужна вся история для восстановления пути).

### Частые вариации

- **Longest Common Substring** — та же таблица, но переход другой: при несовпадении `dp[i][j] = 0` (подстрока обязана быть непрерывной), а ответ — максимум по всей таблице, а не `dp[n][m]`.
- **Edit Distance** — похожая структура DP, но с тремя операциями (вставка/удаление/замена) вместо просто "включить/не включить" символ.
- **Shortest Common Supersequence** — кратчайшая строка, содержащая обе строки как подпоследовательности — длина равна `n + m - LCS(text1, text2)`.
- **Longest Palindromic Subsequence** — LCS строки с самой собой в обратном порядке (`LCS(s, reverse(s))`).
