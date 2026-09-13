## Суть задачи **Least Recently Used**

Реализовать кэш с ограниченной ёмкостью (capacity), который поддерживает:

- `get(key)` — вернуть значение по ключу за `O(1)`, если есть; иначе -1
- `put(key, value)` — добавить/обновить пару за `O(1)`

При превышении capacity — вытеснить **наименее недавно использованный** элемент (Least Recently Used). Обе операции — `get` и `put` — должны считаться "использованием", то есть двигать элемент в начало очереди "свежести".

## Идея структуры

Ни одна структура сама по себе не даёт `O(1)` на всё нужное:

- **Hash map** даёт `O(1)` доступ по ключу, но не хранит порядок "давности использования".
- **Doubly linked list** даёт `O(1)` перемещение элемента в начало/конец списка, но сам по себе не даёт `O(1)` поиска по ключу.

Решение — **комбинация**: hash map (`key → указатель на узел списка`) + doubly linked list (порядок по давности использования, голова = самый свежий, хвост = самый старый). Именно двусвязный список, а не односвязный, — потому что при `get` нужно выдернуть узел из середины списка и переместить в начало за `O(1)`, а для этого нужен доступ к `prev` узла, который есть только в двусвязном списке.


```cpp
#include <iostream>
#include <format>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

namespace {
    class LRUCache {
        struct Node {
            int key;
            int value;
            Node* next;
            Node* prev;

            explicit Node(const int key, const int value):
                key{key},
                value{value},
                next{nullptr},
                prev{nullptr} {}
        };

        static constexpr int DEFAULT_SIZE{10};
        static constexpr int ERR_ABSENT{-1};

        int capacity{0};
        std::unordered_map<int, Node*> cache;
        Node* head{nullptr}; // dummy, сразу за ним — самый свежий
        Node* tail{nullptr}; // dummy, перед ним — самый старый (кандидат на вытеснение)

        std::shared_mutex cache_mutex;

        static void remove(const Node* node) {
            node->prev->next = node->next;
            node->next->prev = node->prev;
        }

        void insert_front(Node* node) const {
            node->next = head->next;
            node->prev = head;
            head->next->prev = node;
            head->next = node;
        }

    public:
        explicit LRUCache(const int capacity)
            : capacity(capacity <= 0 ? DEFAULT_SIZE : capacity) {
            head = new Node{0, 0};
            tail = new Node{0, 0};
            head->next = tail;
            tail->prev = head;
        }

        int get(const int key) {
            std::shared_lock<std::shared_mutex> lock{cache_mutex};

            if (!cache.contains(key))
                return ERR_ABSENT;
            Node* node{cache[key]};
            remove(node);
            insert_front(node);

            return node->value;
        }

        void put(const int key, const int value) {
            std::unique_lock<std::shared_mutex> lock{cache_mutex};

            if (cache.contains(key)) {
                remove(cache[key]);
                delete cache[key];
            }

            const auto node{new Node{key, value}};
            cache[key] = node;
            insert_front(node);

            if (cache.size() <= capacity) return;

            const auto lru = tail->prev;
            remove(lru);
            cache.erase(lru->key);
            delete lru;
        }
    };
}

int main() {
    LRUCache cache(2);

    cache.put(1, 10);
    cache.put(2, 20);
    std::cout << "get(1): " << cache.get(1) << std::endl; // 10, элемент 1 стал "свежим"

    cache.put(3, 30); // capacity=2 превышена -> вытесняется наименее недавно использованный (ключ 2)
    std::cout << "get(2): " << cache.get(2) << std::endl; // -1, потому что был вытеснен

    cache.put(4, 40); // снова превышение -> вытесняется ключ 1 (он не трогался с момента get(1))
    std::cout << "get(1): " << cache.get(1) << std::endl; // -1
    std::cout << "get(3): " << cache.get(3) << std::endl; // 30
    std::cout << "get(4): " << cache.get(4) << std::endl; // 40

    return 0;
}

```

Использование двух dummy-узлов (`head`/`tail`) — не косметика, а способ избавиться от проверок на `nullptr` при вставке/удалении на границах списка (типичный C++-паттерн для двусвязных списков).

Сложность: `get` и `put` — оба честные `O(1)` (amortized, без скрытых `O(n)` где-либо).

## Усложнение "сделай thread-safe"

Это explicit упомянутое интервьюером усложнение, и оно прямо стыкуется с темой "многопоточность" из вакансии. Наивный вариант:

```cpp
class ThreadSafeLRUCache {
    std::mutex mtx;
    LRUCache cache; // как выше
public:
    ThreadSafeLRUCache(int cap) : cache(cap) {}

    int get(int key) {
        std::lock_guard<std::mutex> lock(mtx);
        return cache.get(key);
    }

    void put(int key, int value) {
        std::lock_guard<std::mutex> lock(mtx);
        cache.put(key, value);
    }
};
```

Это корректно, но **сериализует все чтения** — глобальный mutex означает, что даже два параллельных `get()` от разных потоков не могут выполниться одновременно, хотя логически они не конфликтуют по данным. Для read-heavy сценария (а кэш — почти всегда read-heavy: чтений на порядки больше, чем записей) это плохо масштабируется на поток.

На собеседовании для позиции, где explicit требуют "глубоко разбираетесь в lock-free", единственный `mutex` — это **отправная точка**, а не финальный ответ. Ожидаемое развитие разговора:

1. **"Почему `get` тоже требует блокировки, если он вроде бы 'только чтение'?"** — ключевая ловушка вопроса. `get` в LRU **не** read-only с точки зрения структуры: он мутирует список (перемещает узел в начало) как побочный эффект. Значит, `shared_mutex`/`rwlock` тут **не поможет** напрямую — все `get` конкурируют за эксклюзивный доступ к списку, потому что даже "просто чтение" двигает узлы.
2. **"Как тогда сделать более конкурентным?"** — практические направления, которые стоит назвать:
    - **Sharding**: разбить кэш на N независимых shard'ов (например, по `hash(key) % N`), каждый со своим mutex — конкурентные ключи из разных shard'ов не блокируют друг друга. Это самый практичный ответ, и он же — прямая параллель с шардированием индекса, о котором уже шла речь.
    - **Lock-free hash map + отдельная стратегия вытеснения**: например, использовать конкурентную хэш-таблицу (concurrent hash map) для быстрого `O(1)` доступа без блокировок, а порядок "давности" поддерживать приближённо — например, через **CLOCK-алгоритм** (второй шанс) вместо точного LRU: каждый элемент хранит atomic "bit использования", вместо перемещения в списке при каждом `get()` просто выставляется флаг `used=true` атомарно (`compare_exchange` или просто `store`), а вытеснение проходит по кругу и сбрасывает флаги — это жертвует **точностью** LRU-порядка ради снятия необходимости мутировать список на каждое чтение.
    - **Разделить hot path и eviction**: `get()` делает быстрый lock-free/атомарный lookup и помечает "использовано" (без немедленного перемещения в списке), а физическое обновление порядка происходит батчами в фоновом потоке или лениво при следующем `put`.

## Параллель с поисковой платформой

LRU (или его вариации — LFU, ARC, W-TinyLFU) — стандартный компонент кэширования на нескольких уровнях поискового рантайма:

- **Кэш результатов запроса** (query result cache) — повторяющиеся popular-запросы не пересчитываются заново.
- **Кэш posting-листов / сегментов индекса** в памяти — раз чтение с диска/по сети дороже, чем из RAM.
- **Кэш промежуточных вычислений** (например, скоров релевантности для часто встречающихся пар запрос-документ).

Учитывая явное указание "миллионы RPS" в вакансии, наивный global-mutex LRU почти наверняка будет назван интервьюером как **bottleneck**, и ожидание — что вы сами дойдёте до sharding или lock-free approximation, не дожидаясь наводящего вопроса.

