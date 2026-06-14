---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/threads/_|<=]]

## Lock-free структуры данных

Структуры данных, которые обеспечивают потокобезопасность **без мьютексов** — через атомарные операции. Гарантируют прогресс: хотя бы один поток всегда продвигается вперёд.

### Ключевая операция — Compare and Swap (CAS)

```cpp
std::atomic<T> val;

// атомарно: если val == expected → записать desired, вернуть true
// иначе → записать текущее в expected, вернуть false
val.compare_exchange_weak(expected, desired);
val.compare_exchange_strong(expected, desired);
```

- `weak` — может ложно вернуть `false` (spurious failure), использовать в цикле
- `strong` — гарантирован, но медленнее

### Lock-free vs mutex

| |`mutex`|`lock-free`|
|---|---|---|
|Реализация|просто|сложно|
|Дедлок|возможен|невозможен|
|Производительность|хуже при contention|лучше при высокой нагрузке|
|ABA-проблема|нет|есть|

### ABA-проблема

Поток читает значение `A`, другой меняет `A→B→A`. CAS видит `A` и считает что ничего не изменилось — хотя структура уже другая. Решение: `std::atomic<std::pair<T*, size_t>>` — версионный указатель.

### Lock-free счётчик

```cpp
#include <iostream>
#include <thread>
#include <atomic>

std::atomic<int> counter{};

void increment(int _n) {
    for (int i{}; i < _n; ++i) {
        counter.fetch_add(1, std::memory_order_relaxed);
    }
}

int main() {
    {
        std::jthread t0{increment, 1000};
        std::jthread t1{increment, 1000};
    }

    std::cout << "counter: " << counter << "\n";

    return 0;
}
```

```
counter: 2000
```

### Lock-free стек

```cpp
#include <iostream>
#include <memory>
#include <atomic>
#include <optional>
#include <thread>

template<typename T>

class Stack {
    struct Node {
        T value;
        Node* next;
        Node(T v): value{std::move(v)}, next{nullptr} {}
    };

public:
    void push(T _value) {
        Node* new_node = new Node{std::move(_value)};
        new_node->next = header.load();
        // CAS: in case of header did not changed then write new_node
        while (!header.compare_exchange_weak(new_node->next, new_node));
    }

    std::optional<T> pop() {
        Node* old_header = header.load();
        while (
            old_header &&
            !header.compare_exchange_weak(old_header, old_header->next)
        );

        if (!old_header) return std::nullopt;

        T value = std::move(old_header->value);
        delete old_header;

        return value;
    }

private:
    std::atomic<Node*> header{nullptr};
};

int main() {
    Stack<int> stack;

    std::jthread writer0{[&]() {
        for (int i{}; i < 3; i++) {
            stack.push(i);
        }
    }};
    std::jthread writer1{[&]() {
        for (int i{}; i < 3; i++) {
            stack.push(i);
        }
    }};
    std::jthread reader{[&](std::stop_token _stoken) {
        while (!_stoken.stop_requested()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            auto&& taken = stack.pop();
            if (!taken) {
                continue;
            }

            std::cout << "reader: " << taken.value() << "\n";
        }
    }};

    std::this_thread::sleep_for(std::chrono::seconds(1));
    reader.request_stop();

    return 0;
}
```

```
reader: 2
reader: 1
reader: 0
reader: 2
reader: 1
reader: 0
```

### Как работает CAS в push

```
head → [A] → [B]

new_node->next = head  →  new_node → [A] → [B]

CAS(head, new_node->next, new_node):
  если head всё ещё [A] → head = new_node  ✓
  если другой поток изменил head → повторить
```

### memory_order — контроль барьеров памяти

| memory_order | Чтение      | Запись       | Использование                     |
| ------------ | ----------- | ------------ | --------------------------------- |
| `relaxed`    | —           | —            | счётчики, статистика              |
| `consume`    | зависимые   | —            | указатели (на практике = acquire) |
| `acquire`    | барьер вниз | —            | load флага готовности             |
| `release`    | —           | барьер вверх | store флага готовности            |
| `acq_rel`    | барьер вниз | барьер вверх | `fetch_add`, CAS                  |
| `seq_cst`    | полный      | полный       | по умолчанию, максимум гарантий   |

### relaxed — только атомарность, без гарантий порядка

`Используется для счётчиков, где важен итог, а не порядок`

```cpp
#include <iostream>
#include <atomic>
#include <thread>
#include <format>

int main() {
    std::atomic<int> counter{0};

    {
        std::jthread t0{[&]() {
            counter.fetch_add(1, std::memory_order_relaxed);
        }};
        std::jthread t1{[&]() {
            counter.fetch_add(1, std::memory_order_relaxed);
        }};
    }
    std::cout << std::format("result: {}\n", counter.load());

    return 0;
}
```

```
result: 2
```

###  release / acquire — однонаправленный барьер
- `store(release) "публикует" данные`
- `load(acquire)  "видит" всё что было до store`

```cpp
#include <iostream>
#include <atomic>
#include <thread>
#include <format>

int main() {
    std::atomic<bool> ready{};
    int data{};

    {
        std::jthread producer{[&]() {
            data = 42;
            ready.store(true, std::memory_order_release);
        }};
        std::jthread consumer{[&]() {
            while (!ready.load(std::memory_order_acquire));
            std::cout << std::format("data: {}\n", data);
        }};
    }

    return 0;
}
```

```
data: 42
```


### consume — ослабленный acquire
- `только зависимые от load значения упорядочены`
- `на практике компиляторы повышают до acquire`

```cpp
#include <iostream>
#include <atomic>
#include <thread>
#include <format>

int main() {
    std::atomic<int*> ptr{nullptr};
    int data{};

    {
        std::jthread producer{[&]() {
            data = 100;
            ptr.store(new int{42}, std::memory_order_release);
        }};
        std::jthread consumer{[&]() {
            int* p{nullptr};
            while(!(p = ptr.load(std::memory_order_consume)));
            std::cout << std::format("*ptr: {}, data: {}", *p, data);
            delete p;
        }};
    }

    return 0;
}
```

```
*ptr: 42, data: 100
```

### acq_rel — acquire + release одновременно
- `для операций read-modify-write (fetch_add, CAS)`
- `читает с acquire, пишет с release`

```cpp
#include <iostream>
#include <atomic>
#include <thread>
#include <format>

int main() {
    std::atomic<int> flag{};
    int data{};
    int expected{1};

    {
        std::jthread t0{[&]() {
            data = 42;
            flag.fetch_add(expected, std::memory_order_release);
        }};
        std::jthread t1{[&]() {
            if (flag.compare_exchange_strong(expected, 2, std::memory_order_acq_rel)) {
                std::cout << std::format("data: {}\n", data);
            }
        }};
    }

    return 0;
}
```

```
data: 42
```

### seq_cst — полный последовательный порядок
- `все потоки видят операции в одном порядке`
- `используется по умолчанию, самый медленный`

```cpp
#include <iostream>
#include <atomic>
#include <thread>
#include <format>

int main() {
    std::atomic<bool> x{};
    std::atomic<bool> y{};
    std::atomic<int> z{};

    {
        std::jthread t0{[&](){ x.store(true, std::memory_order_seq_cst); }};
        std::jthread t1{[&](){ y.store(true, std::memory_order_seq_cst); }};

        std::jthread t2{[&](){
            while(!x.load(std::memory_order_seq_cst));
            if (y.load(std::memory_order_seq_cst)) {
                ++z;
                std::cout << std::format("T2 z: {}\n", z.load());
            }
        }};

        std::jthread t3{[&](){
            while(!y.load(std::memory_order_seq_cst));
            if (x.load(std::memory_order_seq_cst)) {
                ++z;
                std::cout << std::format("T3 z: {}\n", z.load());
            }
        }};
    }
    std::cout << std::format("z: {}\n", z.load());

    return 0;
}
```

```
T3 z: 1
z: 1
```
