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

---
---
---

```cpp


// ─────────────────────────────────────────────

// ─────────────────────────────────────────────
void example_acq_rel() {
    std::atomic<int> flag{0};
    int data = 0;

    std::jthread t0([&]() {
        data = 42;
        flag.fetch_add(1, std::memory_order_release); // publish data
    });

    std::jthread t1([&]() {
        // CAS с acq_rel: при успехе видит всё до release
        int expected = 1;
        if (flag.compare_exchange_strong(expected, 2, std::memory_order_acq_rel)) {
            assert(data == 42); // гарантировано
        }
    });
}

// ─────────────────────────────────────────────
// 5. seq_cst — полный последовательный порядок
//    все потоки видят операции в одном порядке
//    используется по умолчанию, самый медленный
// ─────────────────────────────────────────────
void example_seq_cst() {
    std::atomic<bool> x{false};
    std::atomic<bool> y{false};
    std::atomic<int>  z{0};

    std::jthread t0([&]() { x.store(true, std::memory_order_seq_cst); });
    std::jthread t1([&]() { y.store(true, std::memory_order_seq_cst); });

    std::jthread t2([&]() {
        while (!x.load(std::memory_order_seq_cst));
        if (y.load(std::memory_order_seq_cst)) ++z;
    });

    std::jthread t3([&]() {
        while (!y.load(std::memory_order_seq_cst));
        if (x.load(std::memory_order_seq_cst)) ++z;
    });
    // гарантия: z >= 1 — хотя бы один из t2/t3 увидит оба флага
}

int main() {
    example_relaxed();
    example_release_acquire();
    example_consume();
    example_acq_rel();
    example_seq_cst();
    std::cout << "all ok\n";
}
```

---





```cpp
// от слабейшего к сильнейшему:
std::memory_order_relaxed  // только атомарность, без порядка
std::memory_order_acquire  // всё после load видит запись до release
std::memory_order_release  // всё до store видно после acquire
std::memory_order_seq_cst  // полный порядок (по умолчанию, самый медленный)
```

```cpp
// producer
data = 42;
flag.store(true, std::memory_order_release);

// consumer
while (!flag.load(std::memory_order_acquire));
std::cout << data; // гарантированно 42
```

---




---
---
### Темы:
- 
- Работа с GUI или сетевыми серверами в многопоточной среде
- Профилирование и отладка многопоточных программ

### Практика:
```cpp
// Написать простой пул потоков с очередью задач
```

---

## 🛠️ Инструменты и среды

| Инструмент | Для чего |
|----------|---------|
| **g++ / clang++ с `-pthread`** | Компиляция многопоточных программ |
| **Valgrind + Helgrind/DRD** | Поиск race conditions |
| **GCC/Clang с `-fsanitize=thread`** | ThreadSanitizer — лучший выбор |
| **IDE: CLion, VS Code, Visual Studio** | Отладка потоков |

---

## 📚 Рекомендуемые источники

### Книги:
- **"C++ Concurrency in Action"** — *Anthony Williams* (лучшая книга по теме)
- **"Effective Modern C++"** — *Scott Meyers* (разделы про concurrency)

### Онлайн:
- [https://en.cppreference.com](https://en.cppreference.com) — официальная документация
- [https://www.modernescpp.com](https://www.modernescpp.com) — отличные статьи по concurrency
- YouTube: поиск по "C++ threads tutorial"

---

## ✅ Советы по обучению

| Совет | Почему |
|------|--------|
| Пишите код каждый день | Многопоточность требует практики |
| Используйте ThreadSanitizer | Находит ошибки, которые вы не увидите сами |
| Начинайте с простого | Не бросайтесь сразу в lock-free программирование |
| Тестируйте на разных платформах | Поведение может отличаться (Linux vs Windows) |
| Избегайте глобальных переменных | Они усложняют тестирование |

---

## 🎯 Финальный проект (по окончании курса)

> **Создать HTTP-сервер (упрощённый), который:**
> - Обрабатывает запросы в отдельных потоках
> - Имеет thread-safe кэш
> - Использует пул потоков
> - Поддерживает асинхронные операции

---

Если хочешь, могу:
- Прислать пошаговые уроки с примерами
- Подготовить тесты по каждой теме
- Показать, как отлаживать deadlock
- Составить таблицу совместимости (C++11, 17, 20)

📌 Просто скажи: "Да, хочу подробный урок по [тема]"!

Удачи в изучении многопоточности! 💪