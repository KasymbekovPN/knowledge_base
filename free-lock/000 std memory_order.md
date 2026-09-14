
# Memory order в C++: happens-before на примерах

## Общая модель

У каждой atomic-операции есть роль в happens-before графе. Важно: memory_order не защищает _что_ видно (это делает atomicity), а _порядок_, в котором изменения становятся видны другим потокам.

## `memory_order_relaxed`

**Гарантия:** только atomicity операции. Никакого упoрядочивания относительно других операций (даже других atomic!) в этом же потоке для стороннего наблюдателя.

```cpp

#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

namespace {
    std::atomic<int> counter{0};

    void increment() {
        counter.fetch_add(1, std::memory_order_relaxed);
    }
}

int main() {

    std::vector<std::thread> threads;
    for (int i{}; i < 50; ++i) {
        threads.emplace_back(increment);
    }

    for (auto& t: threads) t.join();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::cout << counter.load(std::memory_order_relaxed) << std::endl;

    return 0;
}

```

Это годится для счётчиков статистики, где важен только итоговый результат, а не то, в каком порядке видны промежуточные значения. Но:

```cpp
std::atomic<int> x{0}, y{0};

// Поток A
x.store(1, std::memory_order_relaxed);
y.store(1, std::memory_order_relaxed);

// Поток B
if (y.load(std::memory_order_relaxed) == 1) {
    assert(x.load(std::memory_order_relaxed) == 1); // МОЖЕТ УПАСТЬ
}
```

Компилятор и процессор вправе переставить местами `x.store` и `y.store` с точки зрения потока B — relaxed не даёт никакого happens-before между ними.

## `memory_order_release` + `memory_order_acquire`

**Гарантия:** это парная синхронизация. `release`-запись синхронизируется с `acquire`-чтением _того же атомика_, если acquire увидел значение, записанное release'ом (или более позднее в той же цепочке модификаций). Всё, что было записано (в том числе не-атомарные данные) до release в потоке-писателе, гарантированно видно после acquire в потоке-читателе.

Классический паттерн — публикация данных через флаг:

```cpp

#include <atomic>
#include <iostream>
#include <string>
#include <thread>

namespace {

    std::string payload;
    std::atomic<bool> ready{false};

    void producer() {
        payload = "hello world";
        ready.store(true, std::memory_order_release);
    }

    void consumer() {
        while (!ready.load(std::memory_order_acquire)) {}
        std::cout << payload << std::endl;
    }
}

int main() {

    {
        auto t0{std::jthread(producer)};
        auto t1{std::jthread(consumer)};
    }

    return 0;
}

```

Happens-before цепочка: (1) → (2) [program order] → (3) [если acquire видит запись release] → (4). Без release/acquire компилятор/CPU мог бы переставить (1) и (2), либо (3) и (4), и `payload` мог бы оказаться пустым в момент чтения.

Важно: **acquire/release не создают глобального единого порядка** для _всех_ атомиков сразу — только цепочку между конкретной парой release-writer → acquire-reader на конкретной переменной.

## `memory_order_acq_rel`

Используется для read-modify-write операций (`fetch_add`, `compare_exchange`, `exchange`), которые одновременно и читают, и пишут. Acquire-часть защищает от переупорядочивания того, что после операции, до неё; release-часть — от переупорядочивания того, что было до операции, после неё.

```cpp
std::atomic<int> data{0};
std::atomic<bool> flag{false};

// Поток A: строит структуру, потом "передаёт эстафету" через CAS
void transfer() {
    data.store(42, std::memory_order_relaxed);
    int expected = 0;
    // acq_rel: если CAS успешен — работает как release (публикует data)
    flag_counter.compare_exchange_strong(expected, 1, std::memory_order_acq_rel);
}
```

Типичный пример — lock-free стек (Treiber stack), где `compare_exchange` на head-указателе должен и видеть актуальный head (acquire), и публиковать новый узел (release).

## `memory_order_seq_cst`

**Гарантия:** всё то же, что acquire/release, ПЛЮС существует единый глобальный порядок для _всех_ seq_cst-операций во всей программе, с которым согласны все потоки одновременно. Это самая сильная и самая дорогая гарантия (на x86 почти бесплатно за счёт сильной модели памяти, на ARM — ощутимо дороже, нужен full barrier).

Разница с acquire/release видна на классической задаче Dekker/store-buffering:

```cpp
std::atomic<int> x{0}, y{0};
int r1, r2;

// Поток 1
x.store(1, std::memory_order_seq_cst);
r1 = y.load(std::memory_order_seq_cst);

// Поток 2
y.store(1, std::memory_order_seq_cst);
r2 = x.load(std::memory_order_seq_cst);

// С seq_cst НЕВОЗМОЖНО r1 == 0 && r2 == 0 одновременно
```

Если заменить на `memory_order_acq_rel` (release-часть на store, acquire-часть на load), то `r1 == 0 && r2 == 0` уже **возможен** — потому что release/acquire синхронизируют только пары "своя переменная", а не дают общего порядка между независимыми `x` и `y`. Это ключевое отличие, которое часто спрашивают на собеседованиях.

## Практическое правило выбора

| Сценарий                                                                                                            | memory_order                                                                |
| ------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------- |
| Счётчики статистики, метрики без зависимостей от данных                                                             | `relaxed`                                                                   |
| Публикация данных через флаг/указатель (producer-consumer)                                                          | `release` (writer) / `acquire` (reader)                                     |
| CAS-циклы в lock-free структурах (stack, queue)                                                                     | `acq_rel` (успех), `acquire` или `relaxed` (неудача — данные не изменились) |
| Нужен глобальный тотальный порядок между несколькими независимыми атомиками (редко, но бывает в сложных алгоритмах) | `seq_cst`                                                                   |

На практике: **seq_cst — разумный default**, если не профилировали и не уверены, что acquire/release достаточно. Уходить в более слабые ordering'и стоит только там, где профайлер реально показал контеншн на барьерах — это тонкий инструмент, ошибка в выборе ordering даёт баг, который не воспроизводится в 999 запусках из 1000 и не ловится без TSan.
