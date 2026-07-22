[[raw data/cpp/interview/_|<=]]

## Word Ladder

**Условие:** даны `beginWord`, `endWord` и словарь `wordList`. Найти длину кратчайшей цепочки преобразований от `beginWord` до `endWord`, где на каждом шаге меняется ровно одна буква, и каждое промежуточное слово должно присутствовать в `wordList`. Если `endWord` нет в `wordList` или цепочки не существует — вернуть 0. Длина цепочки считается в количестве слов (включая `beginWord` и `endWord`).

### Идея

Это задача на поиск **кратчайшего пути в невзвешенном графе**, где вершины — слова, а ребро между двумя словами существует, если они отличаются ровно в одной букве. Кратчайший путь в невзвешенном графе — классическая область применения **BFS** (в отличие от DFS, который найдёт какой-то путь, но не обязательно кратчайший).

Ключевой практический вопрос — как эффективно находить соседей слова (все слова из словаря, отличающиеся на одну букву), не перебирая весь словарь для каждого слова (что дало бы O(n² · L)). Решение: для текущего слова перебираем **все возможные замены каждой буквы на все 26 букв алфавита** (`L * 26` вариантов, где `L` — длина слова) и проверяем, есть ли получившееся слово в словаре (`unordered_set` — O(1) проверка).

### Решение

```cpp
#include "word_ladder.h"  
  
#include <queue>  
#include <vector>  
#include <string>  
#include <unordered_set>  
#include <iostream>  
#include <format>  
  
namespace word_ladder {  
  
int ladder_len(const std::string& begin_word,  
               const std::string& end_word,  
               const std::vector<std::string>& word_list) {  
  
    std::unordered_set<std::string> dict(word_list.begin(), word_list.end());  
    if (!dict.contains(end_word)) return 0;  
  
    std::queue<std::string> q;  
    q.push(begin_word);  
    dict.erase(begin_word);  
  
    int steps{1};  
    while (!q.empty()) {  
        const int level_size{static_cast<int>(q.size())};  
  
        for (int i{}; i < level_size; ++i) {  
            std::string word{q.front()};  
            q.pop();  
  
            if (word == end_word) return steps;  
  
            for (size_t pos{0}; pos < word.size(); ++pos) {  
                const char original{word[pos]};  
  
                for (char c{'a'}; c <= 'z'; ++c) {  
                    if (c == original) continue;  
  
                    word[pos] = c;  
  
                    if (dict.contains(word)) {  
                        // помечаем как посещённое (не заходить снова)  
                        dict.erase(word);  
                        q.push(word);  
                    }                
                }                // восстанавливаем слово перед следующей позицией  
                word[pos] = original;  
            }        
        }  
        ++steps;  
    }  
    return 0;  
}  
  
void demo() {  
    const std::string begin_word{"hit"};  
    const std::string end_word{"cog"};  
    const std::vector<std::string> word_list{"hot","dot","dog","lot","log","cog"};  
  
    std::cout << std::format("LEN: {}\n", ladder_len(begin_word, end_word, word_list));  
}  
  
}
```

### Разбор

- `dict` — множество слов словаря для O(1) проверки существования; заодно используется как `visited` — удаление слова из `dict` при добавлении в очередь предотвращает повторную обработку (и повторное попадание в очередь).
- BFS по уровням (`levelSize`, как и в Level Order Traversal): каждый уровень — это ещё одна "замена одной буквы", то есть ещё один шаг цепочки. `steps` увеличивается после полной обработки уровня.
- Генерация соседей: для каждой позиции `pos` в слове перебираем все 26 букв алфавита вместо самой буквы — если получившееся слово есть в `dict`, это сосед в графе. `word[pos] = original` в конце — обязательный откат к исходной букве перед переходом к следующей позиции (иначе последующие замены будут строиться поверх уже изменённого слова).
- Проверка `word == endWord` выполняется сразу при извлечении из очереди — как только `endWord` впервые встречен на каком-то уровне, `steps` на этот момент — гарантированно кратчайшая длина пути (свойство BFS: первое достижение вершины — по кратчайшему пути).
- `steps` инициализируется 1, а не 0, — потому что даже "нулевое" преобразование (`beginWord` уже равен `endWord`, вырожденный случай) или единственный шаг должны считать сам `beginWord` как первое слово цепочки.

### Пример

```
beginWord="hit", endWord="cog"
wordList=["hot","dot","dog","lot","log","cog"]

dict={hot,dot,dog,lot,log,cog}, endWord "cog" есть -> продолжаем
q=[hit], dict без "hit" (его и так там не было), steps=1

Уровень 1 (levelSize=1): word="hit"
  pos=0 ('h'): перебор a..z -> "ait","bit",... "hit"(=original,пропуск)... ни одно не в dict, кроме?
    на самом деле "hit" меняя 'h' -> ничего не совпадёт с dict в этом примере
  pos=1 ('i'): "hat","hbt",...,"hot"! -> в dict -> добавляем "hot", dict.erase("hot")
  pos=2 ('t'): "hia","hib",...,"hip"... ничего из dict (в этом словаре нет "hi?" кроме через pos=1)
q=[hot]
steps=2

Уровень 2 (levelSize=1): word="hot"
  pos=0: "aot","bot",...,"dot"! -> добавляем "dot"; "lot"! -> добавляем "lot"
  pos=1: "hat","hbt",... ничего нового
  pos=2: "hoa","hob",... ничего (кроме уже "hot" самого себя, но это original, пропущено)
q=[dot, lot]
steps=3

Уровень 3 (levelSize=2):
  word="dot": pos=0 "aot".. ничего нового ("hot" уже удалено); pos=2: "dog"! -> добавляем "dog"
  word="lot": pos=2: "log"! -> добавляем "log"
q=[dog, log]
steps=4

Уровень 4 (levelSize=2):
  word="dog": pos=1: "cog"! -> добавляем "cog"
  word="log": pos=1: "cog" уже удалён из dict (только что добавлен в очередь) -> пропуск
q=[cog]
steps=5

Уровень 5 (levelSize=1): word="cog" == endWord -> return steps=5
```

**Ответ: 5** (`hit → hot → dot → dog → cog`, 5 слов в цепочке).

### Сложность

- Время: **O(M² · N)**, где `M` — длина слова, `N` — размер словаря: для каждого слова из очереди перебираем `M` позиций × 26 букв, и на каждую генерацию тратим O(M) на создание строки-кандидата (копирование/сравнение).
- Память: **O(M · N)** — под словарь, очередь и промежуточные строки.

### Частые вариации

- **Word Ladder II** — вернуть **все** кратчайшие цепочки преобразований (не только длину) — BFS для нахождения кратчайшей длины + отдельный DFS/backtracking для восстановления путей по построенному BFS-дереву предков.
- **Open the Lock** — тот же паттерн BFS по неявному графу состояний (комбинации замка), где ребро — смена одной цифры на соседнюю.
- **Minimum Genetic Mutation** — практически изоморфная задача Word Ladder, но алфавит из 4 букв (`A,C,G,T`) вместо 26.
