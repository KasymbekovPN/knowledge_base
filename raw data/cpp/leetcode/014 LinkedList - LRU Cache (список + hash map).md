[[raw data/cpp/interview/_|<=]]

## LRU Cache

**Условие:** спроектировать структуру данных LRU (Least Recently Used) Cache с фиксированной вместимостью `capacity`. Должна поддерживать:

- `get(key)` — вернуть значение по ключу за O(1), либо -1, если ключа нет; при этом ключ становится "самым недавно использованным".
- `put(key, value)` — вставить/обновить пару за O(1); если превышена вместимость — вытеснить **наименее недавно использованный** элемент.

### Идея

Нужна структура, которая одновременно даёт: быстрый доступ по ключу (O(1)) и быстрое определение/обновление порядка использования (O(1)). Ни один список сам по себе, ни одна хеш-таблица сама по себе этого не дают — комбинируем:

- **Двусвязный список** — хранит пары `(key, value)` в порядке использования: голова — самый недавно использованный, хвост — самый давний (кандидат на вытеснение). Перемещение узла в начало и удаление с конца — O(1) при наличии указателя на узел.
- **Хеш-таблица** `key → указатель на узел списка` — даёт мгновенный доступ к нужному узлу списка по ключу, без линейного поиска.

При каждом обращении (`get` или `put` к существующему ключу) — узел переносится в начало списка ("обновление свежести"). При вставке нового элемента сверх вместимости — удаляется узел с конца списка (и соответствующая запись из hash map).

`std::list` в C++ — двусвязный список с O(1) вставкой/удалением по итератору и без инвалидации итераторов при таких операциях (в отличие от `std::vector`) — это и позволяет хранить в hash map "стабильные" итераторы на узлы.

### Решение

```cpp
#pragma once  
  
#include <list>  
#include <unordered_map>  
#include <utility>  
  
namespace lru_cache {  
  
    class LRUCache {  
    public:  
        explicit LRUCache(const int capacity): capacity_(capacity) {}  
        int get(const int);  
        void put(const int, const int);  
    private:  
        using ListIt = std::list<std::pair<int, int>>::iterator;  
  
        void touch(ListIt);  
  
        int capacity_;  
        std::list<std::pair<int, int>> order_;  
        std::unordered_map<int, ListIt> index_;  
    };  
    void demo();  
  
}
```

```cpp
#include "lru_cache.h"  
  
#include <iostream>  
#include <format>  
  
namespace lru_cache {  
  
int LRUCache::get(int key) {  
    auto it{index_.find(key)};  
    if (it == index_.end()) {  
        return -1;  
    }    touch(it->second);  
  
    return it->second->second;  
}  
  
void LRUCache::put(int key, int value) {  
    auto it{index_.find(key)};  
  
    if (it != index_.end()) {  
        it->second->second = value;  
        touch(it->second);  
        return;  
    }  
    if (static_cast<int>(order_.size()) >= capacity_) {  
        // вытесняем наименее недавно использованный (хвост списка)  
        auto& lru = order_.back();  
        index_.erase(lru.first);  
        order_.pop_back();  
    }  
    order_.emplace_front(key, value);  
    index_[key] = order_.begin();  
}  
  
void LRUCache::touch(ListIt it) {  
    order_.splice(order_.begin(), order_, it);  
}  
  
void demo() {  
    auto cache = LRUCache(3);  
    cache.put(0, 100);  
    cache.put(1, 101);  
    cache.put(2, 102);  
    cache.put(3, 103);  
  
    cache.get(3);  
    cache.put(4, 104);  
  
    for (int i{0}; i < 5; ++i) {  
        std::cout << std::format("{} <-> {}\n", i, cache.get(i));  
    }}  
  
}
```

### Разбор

- `order_` — список пар `(key, value)`. Значение храним прямо в списке (не только в hash map), чтобы `get` мог сразу вернуть его через итератор, без второго обращения к hash map.
- `index_` — хеш-таблица `key → итератор в order_`. Именно за счёт этого `get` и `put` не требуют линейного поиска нужного узла.
- `touch(it)` — ключевая операция: `std::list::splice` переносит уже существующий узел в начало списка за O(1), **без копирования данных и без инвалидации итераторов** на этот и другие узлы (это специфическое свойство `std::list`, которого нет у `std::vector`).
- В `put` для нового ключа: если вместимость исчерпана — сначала удаляем хвост (`order_.back()`) и его запись из `index_` (обязательно нужен ключ хвоста, чтобы найти его в hash map — поэтому в списке хранится пара, а не только значение), затем вставляем новый элемент в начало (`emplace_front`) и сохраняем его итератор в `index_`.
- Порядок действий важен: `index_.erase(lru.first)` **до** `order_.pop_back()` — иначе `lru` (ссылка на элемент списка) станет висячей после удаления из списка.

### Пример

```
LRUCache cache(2);

cache.put(1, 1);      // order_: [(1,1)]
cache.put(2, 2);      // order_: [(2,2), (1,1)]
cache.get(1);         // -> 1;  order_: [(1,1), (2,2)]  (1 стал самым свежим)
cache.put(3, 3);      // вместимость исчерпана, вытесняем хвост (2,2)
                       // order_: [(3,3), (1,1)]
cache.get(2);         // -> -1 (2 был вытеснен)
cache.put(4, 4);      // вытесняем хвост (1,1)
                       // order_: [(4,4), (3,3)]
cache.get(1);         // -> -1
cache.get(3);         // -> 3
cache.get(4);         // -> 4
```

### Сложность

- Время: **O(1)** амортизированно для `get` и `put` — хеш-таблица даёт O(1) поиск, `splice`/`pop_back`/`emplace_front` у `std::list` — все O(1).
- Память: **O(capacity)** — хранится не более `capacity` элементов одновременно в обеих структурах.

### Частый доп. вопрос: "а можно без std::list, руками?"

На собеседовании иногда просят реализовать двусвязный список вручную (свою структуру `Node* prev, *next`), не полагаясь на `std::list`, чтобы проверить понимание, как именно работает O(1) перемещение/удаление узла. Логика та же: hash map хранит `key → Node*`, узел вручную отцепляется (`prev->next = next; next->prev = prev`) и вставляется в начало между dummy-head и первым реальным узлом (снова dummy-узлы с обеих сторон — head и tail — избавляют от edge cases на границах списка).

### Частые вариации

- **LFU Cache** — вытеснение по частоте использования, а не по недавности; сложнее — нужен доп. слой группировки по частоте (map частот → список ключей с этой частотой).
- **Design a HashMap / HashSet with capacity constraints** — общий паттерн "hash map + вспомогательная структура порядка" встречается и в других design-задачах.
