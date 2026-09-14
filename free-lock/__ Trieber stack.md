
# Полный пример: Treiber stack на `memory_order_acq_rel`

Lock-free stack — канонический пример, где acq_rel нужен по существу (а не просто "на всякий случай"), потому что `compare_exchange` там одновременно читает текущий head (нужен acquire, чтобы видеть актуальное состояние) и публикует новый head (нужен release, чтобы данные внутри нового узла были видны следующему потоку, который его прочитает).

```cpp
#include <atomic>
#include <memory>

template<typename T>
class TreiberStack {
    struct Node {
        T data;
        Node* next;
        Node(T val) : data(std::move(val)), next(nullptr) {}
    };

    std::atomic<Node*> head{nullptr};

public:
    void push(T value) {
        Node* new_node = new Node(std::move(value));

        // (1) relaxed: пока узел никому не виден, порядок не важен
        new_node->next = head.load(std::memory_order_relaxed);

        // (2) CAS: acq_rel
        //   - acquire-часть: если CAS не удался, нам нужно увидеть
        //     актуальный head для следующей попытки
        //   - release-часть: если CAS удался, публикуем new_node —
        //     всё, что записано в него (data, next) в шаге (1),
        //     станет видно потоку, который потом сделает pop()
        while (!head.compare_exchange_weak(
                   new_node->next,      // expected (обновляется при неудаче)
                   new_node,            // desired
                   std::memory_order_acq_rel,   // при успехе
                   std::memory_order_relaxed))  // при неудаче достаточно relaxed
        {
            // new_node->next уже обновлён compare_exchange_weak до
            // актуального head — просто повторяем попытку
        }
    }

    bool pop(T& result) {
        Node* old_head = head.load(std::memory_order_acquire);

        while (old_head != nullptr &&
               !head.compare_exchange_weak(
                   old_head,
                   old_head->next,
                   std::memory_order_acq_rel,   // успех: читаем + публикуем
                   std::memory_order_acquire))  // неудача: нужен свежий head
        {
            // old_head обновлён автоматически, повторяем
        }

        if (old_head == nullptr) return false;

        // (3) Здесь безопасно читать old_head->data — happens-before
        // от push() гарантирует, что данные видны
        result = std::move(old_head->data);
        delete old_head; // УПРОЩЕНИЕ: в реальности тут ABA problem +
                          // use-after-free нужно решать через hazard
                          // pointers или epoch-based reclamation
        return true;
    }
};
```

## Разбор happens-before по шагам

**push():**

1. `new_node->next = head.load(relaxed)` — обычная запись, узел ещё локален для потока, никто его не видит.
2. `compare_exchange_weak(..., acq_rel, relaxed)`:
    - Если **успех** — release-часть гарантирует: всё, что было записано в `new_node` (включая `data` и `next`) до этой точки, "публикуется" вместе с новым значением `head`.
    - Если **неудача** — acquire-часть не нужна для публикации (мы ничего не опубликовали), но нужна, чтобы **сам поток** увидел актуальный `head` для следующей попытки. Поэтому failure-order можно оставить `relaxed` — сам `compare_exchange_weak` в любом случае обновит `expected` актуальным значением по стандарту.

**pop():**

1. `head.load(acquire)` — читаем текущий head, acquire нужен, чтобы увидеть данные того узла, который был опубликован через release в push().
2. `compare_exchange_weak(..., acq_rel, acquire)`:
    - **Успех**: acquire-часть гарантирует, что мы видим содержимое `old_head` (опубликованное push'ем через release) — отсюда безопасность строки (3). Release-часть здесь тоже нужна: если другой поток параллельно читает `head` после нашего pop, он должен увидеть согласованное состояние.
    - **Неудача**: acquire нужен, чтобы получить свежий `head` для повторной попытки (в отличие от push, здесь я поставил `acquire` а не `relaxed` на failure — это чуть консервативнее; на практике многие реализации используют `relaxed` и здесь тоже, если логика ретрая не зависит от увиденных данных, только от значения указателя).

## Почему не `seq_cst` и не просто `acquire`/`release` по отдельности

- **Не просто relaxed**: тогда данные внутри `new_node` могли бы быть не видны другому потоку после успешного pop — гонка на `data`.
- **Не seq_cst**: seq_cst дал бы то же самое happens-before для _этой_ пары операций, но дороже (full memory barrier на ARM/PowerPC), а глобальный тотальный порядок между _разными_ атомиками здесь просто не нужен — у нас только один atomic (`head`), синхронизация полностью локальна к нему.
- **acq_rel именно на CAS, а не отдельно acquire+release**: потому что `compare_exchange` — это единая read-modify-write операция, ей нужна и acquire-семантика (для чтения), и release-семантика (для записи) одновременно, в одной атомарной инструкции.

## Важная оговорка

Этот код **некорректен для продакшена** из-за ABA problem и use-after-free в `delete old_head`: пока поток A читает `old_head->next` внутри цикла CAS, другой поток B может успеть сделать `pop()` этого же узла, `delete` его, аллоцировать новый узел по тому же адресу — и CAS у потока A "успешно" пройдёт по указателю, хотя структура за ним уже другая. Реальная реализация требует hazard pointers, epoch-based reclamation или `std::atomic<std::shared_ptr<Node>>` (C++20) — это отдельная большая тема, которую стоит брать сразу после того, как разберётесь с memory_order на этом уровне.

Хотите разобрать ABA problem с конкретным сценарием интерливинга потоков, или перейти к hazard pointers как решению?