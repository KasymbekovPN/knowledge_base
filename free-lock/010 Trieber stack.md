

# Полный пример: Treiber stack на `memory_order_acq_rel`


```cpp
#include <atomic>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

namespace {

    template <typename T>
    class TreiberStack {
        struct Node {
            T data;
            Node* next;
            explicit Node(T v): data{std::move(v)}, next{nullptr} {}
        };

        std::atomic<Node*> head{nullptr};

    public:
        void push(T value) {
            Node* new_value{new Node{std::move(value)}};

            // (1) relaxed: пока узел никому не виден, порядок не важен
            new_value->next = head.load(std::memory_order_relaxed);

            // (2) CAS: acq_rel
            //   - acquire-часть: если CAS не удался, нам нужно увидеть
            //     актуальный head для следующей попытки
            //   - release-часть: если CAS удался, публикуем new_node —
            //     всё, что записано в него (data, next) в шаге (1),
            //     станет видно потоку, который потом сделает pop()
            while (!head.compare_exchange_weak(
                new_value->next,
                new_value,
                std::memory_order_acq_rel,
                std::memory_order_relaxed)) {
                // new_node->next уже обновлён compare_exchange_weak до
                // актуального head — просто повторяем попытку
            }
        }

        bool pop(T& result) {
            Node* old_head{head.load(std::memory_order_acquire)};
            while (old_head != nullptr && !head.compare_exchange_weak(
                old_head,
                old_head->next,
                std::memory_order_acq_rel,
                std::memory_order_acquire)) {
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

    std::atomic<int> total_pushed{0};
    std::atomic<int> total_popped{0};
    std::atomic<bool> stop_consumers{false};

    void producer(const int thread_id, const int ops_count, TreiberStack<int>& stack) {
        for (int i = 0; i < ops_count; ++i) {
            // Генерируем уникальное значение: ID потока * 10000 + счетчик
            const int value = thread_id * 10000 + i;
            stack.push(value);
            total_pushed.fetch_add(1, std::memory_order_relaxed);

            // Небольшая случайная задержка увеличивает шанс пересечения потоков
            // и проявления ABA-проблемы при удалении/выделении памяти
            if (i % 10 == 0) {
                std::this_thread::yield();
            }
        }
    }

    void consumer(const int ops_count, TreiberStack<int>& stack) {
        int value;
        int local_popped = 0;

        while (local_popped < ops_count) {
            if (stack.pop(value)) {
                local_popped++;
                total_popped.fetch_add(1, std::memory_order_relaxed);
            } else {
                // Если стек пуст, даем другим потокам шанс выполнить push
                std::this_thread::yield();
            }
        }
    }
}


int main(int argc, char *argv[]) {
    std::cout << "Start to stress-test for TreiberStack...\n";

    TreiberStack<int> stack;

    constexpr int NUM_PRODUCERS = 4;
    constexpr int NUM_CONSUMERS = 4;
    constexpr int OPS_PER_THREAD = 5000; // Увеличьте это число, чтобы повысить шанс поймать ABA

    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;

    // 1. Запускаем потоки-потребители (они будут ждать данные)
    for (int i = 0; i < NUM_CONSUMERS; ++i) {
        consumers.emplace_back(consumer, OPS_PER_THREAD, std::ref(stack));
    }

    // 2. Запускаем потоки-производители
    for (int i = 0; i < NUM_PRODUCERS; ++i) {
        producers.emplace_back(producer, i, OPS_PER_THREAD, std::ref(stack));
    }

    // 3. Ожидаем завершения всех потоков
    for (auto& t : producers) {
        t.join();
    }
    for (auto& t : consumers) {
        t.join();
    }

    // 4. Проверка результатов
    std::cout << "\n--- Result ---\n";
    std::cout << "Total push:  " << total_pushed.load() << "\n";
    std::cout << "Total pop:   " << total_popped.load() << "\n";

    // Проверяем, остался ли стек пустым
    int dummy;
    if (!stack.pop(dummy)) {
        std::cout << "Stack is empty\n";
    } else {
        std::cout << "WARN: Stack is not empty.\n";
    }

    // Если числа не сходятся или программа упала с Segmentation Fault,
    // это прямое следствие ABA-проблемы и use-after-free в методе pop().
    if (total_pushed.load() != total_popped.load()) {
        std::cout << "WARN: total_popped != total_pushed\n";
    } else {
        std::cout << "total_popped == total_pushed";
    }

    return 0;
}

```

Lock-free stack — канонический пример, где acq_rel нужен по существу (а не просто "на всякий случай"), потому что `compare_exchange` там одновременно читает текущий head (нужен acquire, чтобы видеть актуальное состояние) и публикует новый head (нужен release, чтобы данные внутри нового узла были видны следующему потоку, который его прочитает).

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

---
---

## Блок 1 — Фундамент: memory model и atomics (3-5 дней)

**Теория:**

- [x] `std::memory_order`: relaxed, acquire, release, acq_rel, seq_cst — что каждый реально гарантирует на уровне happens-before (2026.09.14)
- [x] Разница между atomicity и ordering  (2026.09.14)
- [x] ABA problem и способы борьбы (tagged pointers, hazard pointers, epoch-based reclamation)  (2026.09.15)
- [x] False sharing и cache line padding (`alignas(64)`, `std::hardware_destructive_interference_size`)  (2026.09.15)
- [x] CAS (compare_exchange_weak vs strong) — почему weak предпочтителен в циклах (2026.09.15)

**Практика:**

- [x] Написать spinlock на atomic_flag, замерить под контейнером с perf (2026.09.17)
- [x] Реализовать простой atomic counter с разными memory_order и посмотреть разницу в асме (godbolt) (2026.09.17)
- [x] Разобрать пример ABA на указателях вручную (2026.09.18)

**Источники:** "C++ Concurrency in Action" (Anthony Williams) главы 5, 7; CppCon talks Herb Sutter "atomic<> Weapons"; cppreference по memory_order.

## Блок 2 — Lock-free структуры данных (1-2 недели)

**Порядок изучения (от простого к сложному):**

1. [x] **MPSC queue** (Michael-Scott или на основе intrusive linked list) — уже ближе к реальному broadcast-паттерну (2026.09.18)
2. [x] **MPMC queue** (Dmitry Vyukov's bounded queue — классика, часто спрашивают на собесах) (2026.09.18)
3. [x] Lock-free stack (Treiber stack) — проще MPMC, хорошо иллюстрирует ABA (2026.09.18)

**Для каждой структуры:**

- [ ] Реализовать самому на raw atomics (не подглядывая)
- [ ] Написать stress-test с несколькими потоками + TSan (ThreadSanitizer) — это критично, lock-free код без санитайзера почти невозможно верифицировать
- [ ] Сравнить throughput/latency с mutex-based аналогом (тот же интерфейс, `std::mutex + std::deque`)

**Источники:** Dmitry Vyukov's blog (1024cores.net) — обязательно; "The Art of Multiprocessor Programming" (Herlihy & Shavit) главы про queues/stacks; boost::lockfree как референс-реализация для сверки.

## Блок 3 — Архитектура lock-free чат-сервера (1 неделя)

**Ключевые решения для чата конкретно:**

- [ ] **Модель потоков**: thread-per-connection vs event loop (epoll/io_uring) + пул воркеров — lock-free структуры чаще всего нужны именно во второй модели, как shared-state между воркерами
- [ ] **Message queue на клиента**: SPSC от воркера к connection-writer (один producer — логика чата, один consumer — сетевой поток на отправку)
- [ ] **Broadcast**: тут MPSC/MPMC не всегда лучший выбор — часто эффективнее copy-on-write список подписчиков (RCU-style) + каждому подписчику своя SPSC-очередь, чем один общий lock-free broadcast-канал
- [ ] **Backpressure**: что делать, если consumer медленнее producer — bounded queue + drop policy vs unbounded + memory blowup
- [ ] **Reclamation проблема**: если пользователь отключается, когда безопасно освободить его структуры при lock-free доступе (hazard pointers / epoch-based / RCU)

**Практика:** собрать минимальный прототип — N потоков-писателей кладут сообщения в шину, M потоков-читателей (эмулируют коннекты) их разбирают, с метриками latency/throughput под нагрузкой.

**Источники:** статьи про архитектуру Discord/Slack backend (общие принципы, не lock-free специфика, но полезно для контекста), RCU paper (Paul McKenney), доклады про io_uring + lock-free от CppCon/Meeting C++.

----
---
---
---
---
---
---
---
---
---
---
---
---

###  Lock-free stack (Treiber stack) — проще MPMC, хорошо иллюстрирует ABA

---
---

### Реализовать самому на raw atomics (не подглядывая)

Да, именно так — это пункт из **практики** Блока 2, где нужно самому написать структуру данных на `std::atomic`, не подглядывая в реализации, которые уже разбирали в этом чате. Смысл упражнения — не в том, чтобы получить готовый код, а в том, чтобы вы сами прошли через все ловушки (memory_order, ABA, порядок операций), которые я до этого объяснял на пальцах — тогда они закрепятся, а не останутся "я вроде понял, когда Claude объяснял".

## Что конкретно делать

**Возьмите одну структуру за раз**, начиная с самой простой из списка Блока 2:

1. **SPSC ring buffer** — самое простое, с него и стоит начать
2. Treiber stack
3. MPSC-очередь

Не пытайтесь сразу воспроизвести MPMC Vyukov'а — это самая сложная из показанных, и на ней легче всего скопировать структуру мышления вместо того, чтобы понять её самостоятельно.

## Правила "не подглядывая"

- Закройте эту переписку (или хотя бы не листайте её во время написания)
- Дайте себе конкретное время — например, 45 минут на SPSC ring buffer
- Разрешено: cppreference (описание `std::atomic`, `memory_order`), но не примеры чужих реализаций lock-free структур
- Не разрешено: подсматривать в код, который я писал выше в этом чате, в boost::lockfree, в готовые GitHub-репозитории

## Как понять, что получилось

Ваша реализация должна пройти три проверки — это и есть критерий "работает", а не просто "компилируется":

1. **Функциональный тест**: push N значений одним потоком, pop N значений другим, checksum до и после совпадает
2. **ThreadSanitizer**: `g++ -fsanitize=thread -pthread` — должен быть чист от гонок
3. **Стресс-тест**: не 10 значений, а хотя бы 100 000-1 000 000 — многие баги в lock-free коде не проявляются на маленьких объёмах данных, потому что нужный interleaving потоков просто не успевает произойти

## Что делать, если застряли

Если через 45-60 минут не получается сдвинуться — это нормально, lock-free код сложен именно тем, что интуиция из обычного многопоточного программирования (мьютексы) здесь не работает напрямую. В этом случае:

- Напишите, на каком конкретно месте застряли (например: "не понимаю, что делать в pop, если producer ещё не успел обновить tail") — так я смогу подсказать конкретную точку, а не выдать готовое решение
- Или пришлите свою реализацию, и я укажу баги/гонки без переписывания её за вас — это ближе к code review, чем к готовому ответу, и полезнее для собеседования

Хотите начать с SPSC ring buffer — уточню только интерфейс (какие методы должны быть: `push`/`pop`, `try_push`/`try_pop`, размер как compile-time template параметр или runtime), чтобы вы писали не в вакууме, а к конкретному контракту?

---
---

### Написать stress-test с несколькими потоками + TSan (ThreadSanitizer) — это критично, lock-free код без санитайзера почти невозможно верифицировать

Это отдельный, самостоятельный навык от написания самой структуры — вот генерический шаблон стресс-теста, который вы сможете переиспользовать для **любой** структуры, что напишете сами (SPSC, Treiber stack, что угодно), не подглядывая в её реализацию.

## Универсальный шаблон стресс-теста

```cpp
#include <atomic>
#include <thread>
#include <vector>
#include <iostream>
#include <chrono>
#include <cassert>

// Параметризуем тест над ВАШЕЙ структурой -- сюда подставляете
// свой SPSC/Treiber/что угодно. Тест не знает деталей реализации,
// только интерфейс push/pop.
template<typename Queue>
void stress_test(const char* name, int num_producers, int num_consumers,
                  int items_per_producer) {
    Queue queue; // ваша структура, дефолтный конструктор

    const int total = num_producers * items_per_producer;

    std::atomic<long long> checksum_in{0};
    std::atomic<long long> checksum_out{0};
    std::atomic<int> produced{0};
    std::atomic<int> consumed{0};

    auto start = std::chrono::steady_clock::now();

    std::vector<std::thread> producers;
    for (int p = 0; p < num_producers; ++p) {
        producers.emplace_back([&, p] {
            for (int i = 0; i < items_per_producer; ++i) {
                int value = p * items_per_producer + i;
                queue.push(value); // или ваш метод enqueue/etc
                checksum_in.fetch_add(value, std::memory_order_relaxed);
                produced.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    std::vector<std::thread> consumers;
    for (int c = 0; c < num_consumers; ++c) {
        consumers.emplace_back([&] {
            int value;
            while (consumed.load(std::memory_order_relaxed) < total) {
                if (queue.pop(value)) {
                    checksum_out.fetch_add(value, std::memory_order_relaxed);
                    consumed.fetch_add(1, std::memory_order_relaxed);
                } else {
                    std::this_thread::yield();
                }
            }
        });
    }

    for (auto& t : producers) t.join();
    for (auto& t : consumers) t.join();

    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();

    bool ok = (checksum_in.load() == checksum_out.load())
              && (produced.load() == total)
              && (consumed.load() == total);

    std::cout << "[" << name << "] "
              << (ok ? "PASS" : "FAIL")
              << " (in=" << checksum_in.load()
              << ", out=" << checksum_out.load()
              << ", " << ms << " ms)\n";

    assert(ok && "checksum mismatch -- потерянные или задвоенные элементы");
}

int main() {
    // Подставьте сюда СВОЮ структуру вместо MyQueue
    // stress_test<MyQueue>("SPSC", 1, 1, 1'000'000);
    return 0;
}
```

## Почему именно checksum, а не просто "досчитали до N"

`produced == total && consumed == total` доказывает только количество, но не то, что это были **правильные** значения — теоретически два разных элемента могли быть прочитаны дважды, а третий потерян, и счётчик всё равно сойдётся. Checksum (сумма всех значений) ловит и потери, и задвоения: если хоть один элемент потерян или прочитан дважды, сумма разойдётся почти гарантированно (коллизия суммы теоретически возможна, но для контроля корректности это статистически надёжно).

## Как правильно гонять с TSan

```bash
# Debug-сборка, БЕЗ оптимизаций -- TSan работает надёжнее без -O2,
# и вам нужны точные строки в отчёте, а не переставленный компилятором код
g++ -std=c++20 -O0 -g -fsanitize=thread -pthread stress_test.cpp -o stress_test_tsan

./stress_test_tsan
```

**Важные нюансы, которые часто упускают:**

1. **Один чистый прогон ничего не доказывает.** TSan детектирует гонки, которые реально произошли в конкретном execution — если нужный interleaving потоков не случился именно в этом запуске, TSan промолчит, даже если баг есть. Гоняйте минимум 10-20 раз подряд:
    
    ```bash
    for i in $(seq 1 20); do ./stress_test_tsan || echo "FAILED on run $i"; done
    ```
    
2. **Раздувайте число потоков сверх количества ядер.** 2 потока на 8-ядерной машине почти никогда не покажут contention-баги — увеличьте `num_producers`/`num_consumers` до 16-32, даже если ядер меньше: это форсирует больше context-switch'ей и больше шансов поймать нужный interleaving.
    
3. **TSan не ловит всё.** Он видит только то, что реально исполнилось в конкретном прогоне (dynamic analysis, не static). ABA problem, например, часто **не detectable** через TSan вообще — там нет data race в терминах TSan (обе стороны используют атомики корректно с точки зрения happens-before), баг логический, а не race condition в узком смысле. Для ABA нужен отдельный логический тест (как в демках выше — checksum после конкретного форсированного interleaving).
    
4. **`ThreadSanitizer: data race` в выводе указывает две конкретные строки кода** — первая, где произошла "победившая" операция, вторая — конфликтующая. Если видите гонку на переменной, которую вы считали атомарной — проверьте, не забыли ли обернуть в `std::atomic`, TSan детектирует гонки и на обычных переменных тоже, это частая причина "я же всё сделал через atomic!", когда на самом деле забыли одно поле.
    

## Дополнительно — `-fsanitize=address` не помешает параллельно

Если структура использует `new`/`delete` (Treiber stack, MPSC на linked-list) — есть смысл прогнать отдельно с ASan (не одновременно с TSan, они несовместимы в одной сборке):

```bash
g++ -std=c++20 -O0 -g -fsanitize=address -pthread stress_test.cpp -o stress_test_asan
./stress_test_asan
```

ASan поймает use-after-free (например, если в вашей реализации есть скрытая ABA-дыра, которая приводит к обращению к уже освобождённой памяти) — это дополняет TSan, а не дублирует его.

Когда напишете свою структуру и прогоните через этот шаблон — присылайте, разберём результат вместе, включая любые TSan-репорты, если они появятся.


---
---

### Сравнить throughput/latency с mutex-based аналогом (тот же интерфейс, `std::mutex + std::deque`)


Соберу бенчмарк, сравнивающий Vyukov MPMC с `std::mutex + std::deque` на **одном и том же тестовом харнессе** — throughput (операций/сек) и latency (время одной операции) при разном числе потоков.## Результат

```
producers=1 consumers=1:  MPMC 8.8M ops/sec vs Mutex 7.5M ops/sec  (1.18x)
producers=2 consumers=2:  MPMC 8.7M ops/sec vs Mutex 7.4M ops/sec  (1.17x)
producers=4 consumers=4:  MPMC 8.8M ops/sec vs Mutex 7.5M ops/sec  (1.17x)
producers=8 consumers=8:  MPMC 8.8M ops/sec vs Mutex 7.2M ops/sec  (1.22x)

Latency p50/p99/p999 (ns):
MPMC:  40 / 53  / ~100
Mutex: 48 / 84  / ~230
```

## Важная оговорка про эти цифры

**Разница между 1 и 8 потоками почти не видна** — это артефакт того же ограничения, что и в предыдущих бенчмарках в этом чате: контейнер даёт **1 видимый CPU**, реальной параллельности между потоками нет физически, только чередование планировщиком. На настоящей многоядерной машине картина при росте потоков должна разойтись гораздо сильнее — контеншн на mutex деградирует резче, чем per-slot contention в MPMC, особенно после исчерпания числа физических ядер.

## Что тем не менее видно уже здесь

**p999 latency — самая показательная метрика.** У mutex-варианта p999 (263 ns) почти в 3 раза выше p50 (48 ns) — это хвост, вызванный конкретными "неудачливыми" операциями, которые попали на contended lock и ушли в syscall (futex wait) вместо busy-spin. У lock-free версии p999 (67-127 ns) гораздо ближе к p50 (40 ns) — меньше "выбросов", потому что там нет перехода в kernel-space при contention, только CAS-retry в user-space.

**Это ключевое практическое отличие лока от lock-free**, которое стоит понимать для собеседования: разница не столько в среднем throughput (он может быть сравним, особенно при низком contention — что и видно здесь), сколько в **предсказуемости latency**. Mutex может внезапно "утащить" операцию в десятки микросекунд из-за context switch на busy-системе (что не воспроизводится в этом synthetic-бенчмарке, где нет реальной конкуренции за CPU), тогда как lock-free структура в худшем случае просто крутит CAS-цикл дольше — без syscall overhead.

## Что стоит прогнать на вашей реальной многоядерной машине для честной картины

```bash
g++ -std=c++20 -O2 -pthread mpmc_vs_mutex.cpp -o mpmc_vs_mutex
./mpmc_vs_mutex
```

Ожидаемое отличие от того, что видно здесь: при 8+ потоках на 4-8-ядерной машине mutex-версия должна начать проседать заметнее — contention на единственном mutex растёт линейно с числом потоков, тогда как MPMC деградирует медленнее благодаря per-slot независимости. Также стоит попробовать **асимметричный** сценарий — 1 producer / 8 consumer (или наоборот), это ближе к реальному чат-паттерну (много читателей одной broadcast-очереди) и может показать разницу ярче, чем симметричный N/N.

Это закрывает последний пункт практики Блока 2. Готовы двигаться в Блок 3 — архитектуру чат-сервера целиком (модель потоков, broadcast, backpressure), собирая воедино всё, что разобрали?


```cpp
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <deque>
#include <mutex>
#include <cassert>
#include <algorithm>
#include <numeric>

// ============================================================
// MPMC Vyukov (тот же код, что и раньше)
// ============================================================
template<typename T>
class MPMCQueue {
    struct Cell { std::atomic<size_t> sequence; T data; };
    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> enqueue_pos_;
    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> dequeue_pos_;
    Cell* buffer_;
    size_t buffer_mask_;
public:
    explicit MPMCQueue(size_t capacity)
        : buffer_(new Cell[capacity]), buffer_mask_(capacity - 1) {
        for (size_t i = 0; i < capacity; ++i)
            buffer_[i].sequence.store(i, std::memory_order_relaxed);
        enqueue_pos_.store(0, std::memory_order_relaxed);
        dequeue_pos_.store(0, std::memory_order_relaxed);
    }
    ~MPMCQueue() { delete[] buffer_; }

    bool push(T value) { // единый интерфейс с mutex-версией: push/pop
        Cell* cell; size_t pos = enqueue_pos_.load(std::memory_order_relaxed);
        for (;;) {
            cell = &buffer_[pos & buffer_mask_];
            size_t seq = cell->sequence.load(std::memory_order_acquire);
            intptr_t dif = (intptr_t)seq - (intptr_t)pos;
            if (dif == 0) {
                if (enqueue_pos_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) break;
            } else if (dif < 0) return false;
            else pos = enqueue_pos_.load(std::memory_order_relaxed);
        }
        cell->data = std::move(value);
        cell->sequence.store(pos + 1, std::memory_order_release);
        return true;
    }
    bool pop(T& result) {
        Cell* cell; size_t pos = dequeue_pos_.load(std::memory_order_relaxed);
        for (;;) {
            cell = &buffer_[pos & buffer_mask_];
            size_t seq = cell->sequence.load(std::memory_order_acquire);
            intptr_t dif = (intptr_t)seq - (intptr_t)(pos + 1);
            if (dif == 0) {
                if (dequeue_pos_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) break;
            } else if (dif < 0) return false;
            else pos = dequeue_pos_.load(std::memory_order_relaxed);
        }
        result = std::move(cell->data);
        cell->sequence.store(pos + buffer_mask_ + 1, std::memory_order_release);
        return true;
    }
};

// ============================================================
// Mutex-based аналог -- ТОТ ЖЕ интерфейс push/pop
// ============================================================
template<typename T>
class MutexQueue {
    mutable std::mutex mtx_;
    std::deque<T> deque_;
    size_t capacity_;
public:
    explicit MutexQueue(size_t capacity) : capacity_(capacity) {}

    bool push(T value) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (deque_.size() >= capacity_) return false;
        deque_.push_back(std::move(value));
        return true;
    }
    bool pop(T& result) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (deque_.empty()) return false;
        result = std::move(deque_.front());
        deque_.pop_front();
        return true;
    }
};

// ============================================================
// Общий бенчмарк-харнесс: throughput + latency percentiles
// ============================================================
struct BenchResult {
    double throughput_ops_sec;
    double p50_ns, p99_ns, p999_ns;
};

template<typename Queue>
BenchResult run_bench(int num_producers, int num_consumers,
                       int items_per_producer, size_t capacity) {
    Queue queue(capacity);
    const int total = num_producers * items_per_producer;

    std::atomic<int> consumed{0};
    std::atomic<long long> checksum_in{0}, checksum_out{0};

    std::vector<std::vector<long long>> latencies_per_thread(num_producers);
    for (auto& v : latencies_per_thread) v.reserve(items_per_producer);

    auto start = std::chrono::steady_clock::now();

    std::vector<std::thread> producers;
    for (int p = 0; p < num_producers; ++p) {
        producers.emplace_back([&, p] {
            auto& lat = latencies_per_thread[p];
            for (int i = 0; i < items_per_producer; ++i) {
                int value = p * items_per_producer + i;
                auto t0 = std::chrono::steady_clock::now();
                while (!queue.push(value)) std::this_thread::yield();
                auto t1 = std::chrono::steady_clock::now();
                lat.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count());
                checksum_in.fetch_add(value, std::memory_order_relaxed);
            }
        });
    }

    std::vector<std::thread> consumers;
    for (int c = 0; c < num_consumers; ++c) {
        consumers.emplace_back([&] {
            int value;
            while (consumed.load(std::memory_order_relaxed) < total) {
                if (queue.pop(value)) {
                    checksum_out.fetch_add(value, std::memory_order_relaxed);
                    consumed.fetch_add(1, std::memory_order_relaxed);
                } else {
                    std::this_thread::yield();
                }
            }
        });
    }

    for (auto& t : producers) t.join();
    for (auto& t : consumers) t.join();

    auto end = std::chrono::steady_clock::now();
    double sec = std::chrono::duration<double>(end - start).count();

    assert(checksum_in.load() == checksum_out.load() && "checksum mismatch!");

    std::vector<long long> all_lat;
    for (auto& v : latencies_per_thread)
        all_lat.insert(all_lat.end(), v.begin(), v.end());
    std::sort(all_lat.begin(), all_lat.end());

    auto percentile = [&](double p) -> double {
        size_t idx = static_cast<size_t>(p * (all_lat.size() - 1));
        return static_cast<double>(all_lat[idx]);
    };

    BenchResult r;
    r.throughput_ops_sec = total / sec;
    r.p50_ns = percentile(0.50);
    r.p99_ns = percentile(0.99);
    r.p999_ns = percentile(0.999);
    return r;
}

void print_result(const char* name, const BenchResult& r) {
    std::cout << name << ":\n"
              << "  throughput: " << static_cast<long long>(r.throughput_ops_sec) << " ops/sec\n"
              << "  latency p50:  " << r.p50_ns << " ns\n"
              << "  latency p99:  " << r.p99_ns << " ns\n"
              << "  latency p999: " << r.p999_ns << " ns\n";
}

int main() {
    constexpr size_t CAPACITY = 4096;
    constexpr int ITEMS = 200'000;

    for (auto [producers, consumers] : {std::pair{1,1}, std::pair{2,2}, std::pair{4,4}, std::pair{8,8}}) {
        std::cout << "\n===== producers=" << producers << " consumers=" << consumers << " =====\n";
        auto lockfree = run_bench<MPMCQueue<int>>(producers, consumers, ITEMS, CAPACITY);
        auto mutex_based = run_bench<MutexQueue<int>>(producers, consumers, ITEMS, CAPACITY);
        print_result("MPMC (lock-free)", lockfree);
        print_result("Mutex + deque   ", mutex_based);
        std::cout << "  speedup (throughput): " << (lockfree.throughput_ops_sec / mutex_based.throughput_ops_sec) << "x\n";
    }

    return 0;
}
```

```
g++ -std=c++20 -O2 -pthread mpmc_vs_mutex.cpp -o mpmc_vs_mutex
./mpmc_vs_mutex
```

```
g++ -std=c++20 -O0 -g -fsanitize=thread -pthread mpmc_vs_mutex.cpp -o mpmc_vs_mutex_tsan
./mpmc_vs_mutex_tsan
```


---
---

### **Модель потоков**: thread-per-connection vs event loop (epoll/io_uring) + пул воркеров — lock-free структуры чаще всего нужны именно во второй модели, как shared-state между воркерами

Соберу рабочий прототип: epoll-реактор в одном потоке принимает соединения и читает данные, кладёт задачи в общую lock-free MPMC-очередь, пул воркеров разбирает и обрабатывает — это и есть shared state между воркерами, о котором речь.Работает: все 20 тестовых клиентов подключились по loopback, получили ответы от воркеров, все 20 задач обработаны. Warning про `write()` — безобидный (не проверяем частичную запись, для демо неважно).

# Thread-per-connection vs Event Loop + Worker Pool

## Модель 1: Thread-per-connection

```
Клиент 1 → Поток 1 (блокирующий read/write)
Клиент 2 → Поток 2 (блокирующий read/write)
...
Клиент N → Поток N
```

Каждое соединение — свой ОС-поток, который блокируется на `read()`/`write()`. Просто в коде (обычный последовательный стиль на поток), но:

- **Не масштабируется** — тысячи соединений = тысячи потоков, каждый со своим стеком (обычно 1-8 МБ) и накладными расходами планировщика ОС на context switch.
- **Shared state между потоками нужен редко** — если у каждого клиента своя изолированная логика (например, простой request-response без взаимодействия между клиентами), потокам почти нечего делить. Там, где всё же нужно (общий счётчик онлайн-пользователей, broadcast), достаточно обычного `std::mutex` — редких обращений не хватит, чтобы contention на локе стал узким местом.

**Именно поэтому lock-free здесь обычно избыточен**: конкуренция за shared state пропорциональна частоте обращений к нему, а не количеству потоков как таковому — при тысячах простаивающих на blocking I/O потоков реальный contention на разделяемых структурах может быть низким.

## Модель 2: Event loop + worker pool

```
                    ┌─────────────┐
Клиенты 1..N  →     │ epoll/io_uring│  (1 поток, ставит fd на epoll_wait)
                    └──────┬──────┘
                           │ push() -- lock-free
                           ▼
                 ┌──────────────────┐
                 │  MPMC task queue  │  <-- SHARED STATE
                 └─────────┬────────┘
                    ┌───────┼───────┐
                    ▼       ▼       ▼
                Worker 1  Worker 2  Worker N   (pop(), обрабатывают)
```

Один (или несколько) поток-реактор мультиплексирует **все** соединения через `epoll`/`io_uring` — не блокируется на конкретном клиенте, а ждёт "готовности" сразу у многих fd. Реальная обработка (бизнес-логика) уходит в пул воркеров.

**Здесь lock-free почти обязателен**, и вот почему — это ключевая часть ответа на ваш вопрос:

Реактор — это **единственная точка**, через которую проходят события со всех соединений разом. Если передача задачи от реактора воркеру идёт через `std::mutex`, и воркер (или другой поток) в этот момент держит лок дольше обычного — реактор блокируется на `mutex::lock()`, а значит **не может вернуться к `epoll_wait()`**, а значит **все остальные клиенты замирают**, даже если их данные уже готовы и просто ждут, пока реактор освободится. Один медленный контеншн — и деградация касается всех соединений одновременно, а не одного.

С lock-free `push()` (как в MPMC выше) реактор в худшем случае прокрутит несколько итераций CAS-цикла — микросекунды, не блокировка на неопределённое время из-за чужого потока.

## Разбор ключевого места в коде

```cpp
// Реактор -- единственный поток, крутящий epoll_wait
ssize_t n_read = read(fd, buf, sizeof(buf));
Task task{fd, std::string(buf, n_read)};
if (!task_queue_.push(std::move(task))) {
    // backpressure -- НЕ блокируемся, а сигнализируем и продолжаем
    std::cerr << "queue full, dropping\n";
}
// сразу возвращаемся в epoll_wait -- ни один клиент не ждёт из-за другого
```

```cpp
// N воркеров -- КОНКУРЕНТНО читают из ОДНОЙ и той же очереди
void worker_loop(...) {
    while (...) {
        if (queue.pop(task)) { /* обработка */ }
    }
}
```

Именно **множественность consumer'ов** над **общей** очередью — то, ради чего здесь нужен MPMC, а не MPSC (с MPSC пришлось бы городить отдельную очередь на каждого воркера и балансировку между ними вручную).

## Что стоит доработать для реального прода

- **Backpressure сейчас — просто drop с логом.** В реальности стоит либо динамически растить `capacity`, либо явно отвечать клиенту "перегружен, попробуйте позже", либо (для чата) применять policy "drop oldest" для менее критичных сообщений (presence-обновления) и не дропать критичные (сами сообщения чата).
- **`write()` внутри воркера — потенциальная гонка**, если два воркера одновременно обрабатывают задачи для одного и того же `fd` (в демо это не проявляется, т.к. каждый клиент шлёт одно сообщение, но в реальном чате один и тот же коннект может получить несколько сообщений почти одновременно от разных воркеров). Это ровно то место, где нужна **per-client MPSC-очередь на запись** (Блок 3, следующий пункт) — вместо прямого `write()` из воркера, воркер кладёт исходящее сообщение в MPSC-очередь конкретного клиента, а отдельный writer-поток (или сам реактор через `EPOLLOUT`) её вычитывает и пишет в сокет **строго последовательно**.
- **`io_uring`** решил бы то же самое, что делает epoll здесь, но асинхронно и без syscall на каждый `read`/`write` — концептуально роль lock-free очереди между реактором и воркерами не меняется, меняется только механизм получения событий готовности.

Готовы перейти к следующему пункту Блока 3 — **per-client MPSC-очереди на запись + broadcast через copy-on-write список подписчиков**, который как раз закрывает описанную выше гонку на `write()`?

```cpp
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>
#include <cstring>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <chrono>
#include <arpa/inet.h>

// ============================================================
// MPMC Vyukov -- та же реализация, теперь как SHARED STATE
// между epoll-реактором (producer) и пулом воркеров (consumers).
// ============================================================
template<typename T>
class MPMCQueue {
    struct Cell { std::atomic<size_t> sequence; T data; };
    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> enqueue_pos_;
    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> dequeue_pos_;
    Cell* buffer_;
    size_t buffer_mask_;
public:
    explicit MPMCQueue(size_t capacity)
        : buffer_(new Cell[capacity]), buffer_mask_(capacity - 1) {
        for (size_t i = 0; i < capacity; ++i)
            buffer_[i].sequence.store(i, std::memory_order_relaxed);
        enqueue_pos_.store(0, std::memory_order_relaxed);
        dequeue_pos_.store(0, std::memory_order_relaxed);
    }
    ~MPMCQueue() { delete[] buffer_; }
    bool push(T value) {
        Cell* cell; size_t pos = enqueue_pos_.load(std::memory_order_relaxed);
        for (;;) {
            cell = &buffer_[pos & buffer_mask_];
            size_t seq = cell->sequence.load(std::memory_order_acquire);
            intptr_t dif = (intptr_t)seq - (intptr_t)pos;
            if (dif == 0) { if (enqueue_pos_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) break; }
            else if (dif < 0) return false;
            else pos = enqueue_pos_.load(std::memory_order_relaxed);
        }
        cell->data = std::move(value);
        cell->sequence.store(pos + 1, std::memory_order_release);
        return true;
    }
    bool pop(T& result) {
        Cell* cell; size_t pos = dequeue_pos_.load(std::memory_order_relaxed);
        for (;;) {
            cell = &buffer_[pos & buffer_mask_];
            size_t seq = cell->sequence.load(std::memory_order_acquire);
            intptr_t dif = (intptr_t)seq - (intptr_t)(pos + 1);
            if (dif == 0) { if (dequeue_pos_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) break; }
            else if (dif < 0) return false;
            else pos = dequeue_pos_.load(std::memory_order_relaxed);
        }
        result = std::move(cell->data);
        cell->sequence.store(pos + buffer_mask_ + 1, std::memory_order_release);
        return true;
    }
};

// Задача, которую реактор кладёт в очередь для воркеров.
struct Task {
    int client_fd;
    std::string data;
};

// ============================================================
// EPOLL REACTOR -- ОДИН поток, никогда не блокируется на I/O
// дольше, чем epoll_wait.
// ============================================================
class EpollReactor {
    int epoll_fd_;
    int listen_fd_;
    MPMCQueue<Task>& task_queue_;
    std::atomic<bool>& running_;

    static void set_nonblocking(int fd) {
        int flags = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }

public:
    EpollReactor(int port, MPMCQueue<Task>& queue, std::atomic<bool>& running)
        : task_queue_(queue), running_(running) {
        listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        int opt = 1;
        setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        set_nonblocking(listen_fd_);

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);
        bind(listen_fd_, (sockaddr*)&addr, sizeof(addr));
        listen(listen_fd_, 128);

        epoll_fd_ = epoll_create1(0);
        epoll_event ev{};
        ev.events = EPOLLIN;
        ev.data.fd = listen_fd_;
        epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listen_fd_, &ev);
    }

    void run() {
        constexpr int MAX_EVENTS = 64;
        epoll_event events[MAX_EVENTS];

        while (running_.load(std::memory_order_relaxed)) {
            int n = epoll_wait(epoll_fd_, events, MAX_EVENTS, 100);

            for (int i = 0; i < n; ++i) {
                if (events[i].data.fd == listen_fd_) {
                    int client_fd = accept(listen_fd_, nullptr, nullptr);
                    if (client_fd < 0) continue;
                    set_nonblocking(client_fd);
                    epoll_event cev{};
                    cev.events = EPOLLIN;
                    cev.data.fd = client_fd;
                    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_fd, &cev);
                } else {
                    int fd = events[i].data.fd;
                    char buf[4096];
                    ssize_t n_read = read(fd, buf, sizeof(buf));
                    if (n_read <= 0) {
                        epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
                        close(fd);
                        continue;
                    }
                    Task task{fd, std::string(buf, n_read)};
                    if (!task_queue_.push(std::move(task))) {
                        std::cerr << "[reactor] task queue full, dropping message from fd=" << fd << "\n";
                    }
                }
            }
        }
        close(listen_fd_);
        close(epoll_fd_);
    }
};

// ============================================================
// Worker pool -- N потоков разбирают ОБЩУЮ lock-free очередь.
// ============================================================
void worker_loop(int worker_id, MPMCQueue<Task>& queue, std::atomic<bool>& running,
                  std::atomic<long long>& processed_count) {
    Task task;
    while (running.load(std::memory_order_relaxed) || true) {
        if (queue.pop(task)) {
            std::string response = "[worker " + std::to_string(worker_id) + "] echo: " + task.data;
            ssize_t written = write(task.client_fd, response.data(), response.size());
            (void)written;
            processed_count.fetch_add(1, std::memory_order_relaxed);
        } else {
            if (!running.load(std::memory_order_relaxed)) break;
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    }
}

int main() {
    constexpr int PORT = 18888;
    constexpr int NUM_WORKERS = 4;
    constexpr int NUM_TEST_CLIENTS = 20;

    MPMCQueue<Task> task_queue(1024);
    std::atomic<bool> running{true};
    std::atomic<long long> processed_count{0};

    EpollReactor reactor(PORT, task_queue, running);
    std::thread reactor_thread([&] { reactor.run(); });

    std::vector<std::thread> workers;
    for (int w = 0; w < NUM_WORKERS; ++w) {
        workers.emplace_back(worker_loop, w, std::ref(task_queue), std::ref(running), std::ref(processed_count));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::vector<std::thread> test_clients;
    std::atomic<int> success_count{0};
    for (int c = 0; c < NUM_TEST_CLIENTS; ++c) {
        test_clients.emplace_back([&, c] {
            int sock = socket(AF_INET, SOCK_STREAM, 0);
            sockaddr_in addr{};
            addr.sin_family = AF_INET;
            addr.sin_port = htons(PORT);
            inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

            if (connect(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
                std::cerr << "client " << c << " connect failed\n";
                close(sock);
                return;
            }
            std::string msg = "hello from client " + std::to_string(c);
            ssize_t written = write(sock, msg.data(), msg.size());
            (void)written;

            char buf[256] = {};
            ssize_t n = read(sock, buf, sizeof(buf) - 1);
            if (n > 0) {
                success_count.fetch_add(1, std::memory_order_relaxed);
            }
            close(sock);
        });
    }
    for (auto& t : test_clients) t.join();

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    running.store(false, std::memory_order_relaxed);

    reactor_thread.join();
    for (auto& t : workers) t.join();

    std::cout << "Успешных клиентов (получили ответ): " << success_count.load()
              << " / " << NUM_TEST_CLIENTS << "\n";
    std::cout << "Задач обработано воркерами: " << processed_count.load() << "\n";

    return 0;
}
```

```
g++ -std=c++20 -O2 -pthread event_loop_demo.cpp -o event_loop_demo
./event_loop_demo
```

---
---

### **Message queue на клиента**: SPSC от воркера к connection-writer (один producer — логика чата, один consumer — сетевой поток на отправку)

Реализую полную архитектуру: sharding клиентов по воркерам (гарантирует single producer на каждую per-client очередь) + SPSC-очередь на каждого клиента + отдельный writer-поток как единственный consumer.

Работает: 20/20 клиентов, воркеры и writer обработали по 20 сообщений. Уберу неиспользуемую функцию `worker_loop` (в `main` логику пришлось инлайнить лямбдой из-за регистрации клиента) — почищу перед финальным показом.

```cpp
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <cstring>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <chrono>
#include <new>

// ============================================================
// SPSC ring buffer -- простейшая lock-free очередь, Single
// Producer Single Consumer. Никакого CAS вообще не нужно:
// producer владеет ТОЛЬКО write_pos_, consumer -- ТОЛЬКО
// read_pos_, каждый читает чужую позицию только через atomic
// load для синхронизации видимости (happens-before), но никогда
// не пишет в чужую -- поэтому конкуренции за одну и ту же
// атомарную переменную с двух сторон просто нет.
// ============================================================
template<typename T, size_t Capacity>
class SPSCQueue {
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be power of 2");

    alignas(std::hardware_destructive_interference_size)
        std::atomic<size_t> write_pos_{0}; // пишет ТОЛЬКО producer
    alignas(std::hardware_destructive_interference_size)
        std::atomic<size_t> read_pos_{0};  // пишет ТОЛЬКО consumer

    T buffer_[Capacity];

public:
    bool push(T value) {
        size_t w = write_pos_.load(std::memory_order_relaxed);
        size_t r = read_pos_.load(std::memory_order_acquire);
        if (w - r >= Capacity) return false;

        buffer_[w & (Capacity - 1)] = std::move(value);
        write_pos_.store(w + 1, std::memory_order_release);
        return true;
    }

    bool pop(T& result) {
        size_t r = read_pos_.load(std::memory_order_relaxed);
        size_t w = write_pos_.load(std::memory_order_acquire);
        if (r == w) return false;

        result = std::move(buffer_[r & (Capacity - 1)]);
        read_pos_.store(r + 1, std::memory_order_release);
        return true;
    }
};

// ============================================================
// ClientConnection: несёт per-client исходящую SPSC-очередь.
// Producer этой очереди -- ВСЕГДА один конкретный воркер
// (гарантируется sharding'ом по fd ниже), consumer -- ВСЕГДА
// один writer-поток.
// ============================================================
struct ClientConnection {
    int fd;
    SPSCQueue<std::string, 256> outbox;
    std::atomic<bool> active{true};
};

struct Task { int client_fd; std::string data; };

// ============================================================
// Реактор: принимает соединения, читает данные, ШАРДИРУЕТ
// по fd между воркерами -- КАЖДЫЙ fd всегда попадает к ОДНОМУ
// и тому же воркеру.
// ============================================================
class EpollReactor {
    int epoll_fd_, listen_fd_;
    std::vector<SPSCQueue<Task, 1024>*>& worker_queues_;
    std::atomic<bool>& running_;
    int num_workers_;

    static void set_nonblocking(int fd) {
        int flags = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }

public:
    EpollReactor(int port, std::vector<SPSCQueue<Task, 1024>*>& wq, std::atomic<bool>& running)
        : worker_queues_(wq), running_(running), num_workers_(wq.size()) {
        listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        int opt = 1;
        setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        set_nonblocking(listen_fd_);
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);
        bind(listen_fd_, (sockaddr*)&addr, sizeof(addr));
        listen(listen_fd_, 128);
        epoll_fd_ = epoll_create1(0);
        epoll_event ev{}; ev.events = EPOLLIN; ev.data.fd = listen_fd_;
        epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listen_fd_, &ev);
    }

    void run() {
        constexpr int MAX_EVENTS = 64;
        epoll_event events[MAX_EVENTS];
        while (running_.load(std::memory_order_relaxed)) {
            int n = epoll_wait(epoll_fd_, events, MAX_EVENTS, 100);
            for (int i = 0; i < n; ++i) {
                if (events[i].data.fd == listen_fd_) {
                    int client_fd = accept(listen_fd_, nullptr, nullptr);
                    if (client_fd < 0) continue;
                    set_nonblocking(client_fd);
                    epoll_event cev{}; cev.events = EPOLLIN; cev.data.fd = client_fd;
                    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_fd, &cev);
                } else {
                    int fd = events[i].data.fd;
                    char buf[4096];
                    ssize_t n_read = read(fd, buf, sizeof(buf));
                    if (n_read <= 0) {
                        epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
                        continue;
                    }
                    // SHARDING: один и тот же fd ВСЕГДА идёт к одному воркеру.
                    int worker_idx = fd % num_workers_;
                    Task task{fd, std::string(buf, n_read)};
                    if (!worker_queues_[worker_idx]->push(std::move(task))) {
                        std::cerr << "[reactor] worker " << worker_idx << " queue full, dropping\n";
                    }
                }
            }
        }
        close(listen_fd_);
        close(epoll_fd_);
    }
};

// ============================================================
// Worker: читает СВОЮ SPSC-очередь задач (реактор -- единственный
// producer), обрабатывает, и пишет ответ в SPSC outbox КОНКРЕТНОГО
// клиента. Поскольку fd закреплён за этим воркером (sharding),
// воркер -- ЕДИНСТВЕННЫЙ producer для outbox'а этого клиента.
// ============================================================
void worker_loop(int worker_id, SPSCQueue<Task, 1024>& input_queue,
                  std::unordered_map<int, std::shared_ptr<ClientConnection>>& clients,
                  std::mutex& clients_mutex,
                  std::atomic<bool>& running, std::atomic<long long>& processed) {
    Task task;
    while (running.load(std::memory_order_relaxed) || true) {
        if (input_queue.pop(task)) {
            std::shared_ptr<ClientConnection> conn;
            {
                std::lock_guard<std::mutex> lock(clients_mutex);
                auto it = clients.find(task.client_fd);
                if (it == clients.end()) {
                    conn = std::make_shared<ClientConnection>();
                    conn->fd = task.client_fd;
                    clients[task.client_fd] = conn;
                } else {
                    conn = it->second;
                }
            }

            std::string response = "[worker " + std::to_string(worker_id) + "] echo: " + task.data;
            if (!conn->outbox.push(response)) {
                std::cerr << "[worker " << worker_id << "] outbox full for fd="
                          << task.client_fd << ", dropping\n";
            }
            processed.fetch_add(1, std::memory_order_relaxed);
        } else {
            if (!running.load(std::memory_order_relaxed)) break;
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    }
}

// ============================================================
// Writer: ЕДИНСТВЕННЫЙ поток на всё приложение, вычитывающий
// ВСЕ per-client outbox'ы и реально пишущий в сокеты.
// ============================================================
void writer_loop(std::unordered_map<int, std::shared_ptr<ClientConnection>>& clients,
                  std::mutex& clients_mutex, std::atomic<bool>& running,
                  std::atomic<long long>& written_count) {
    while (running.load(std::memory_order_relaxed) || true) {
        bool did_work = false;
        std::vector<std::shared_ptr<ClientConnection>> snapshot;
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            snapshot.reserve(clients.size());
            for (auto& [fd, conn] : clients) snapshot.push_back(conn);
        }

        for (auto& conn : snapshot) {
            std::string msg;
            while (conn->outbox.pop(msg)) {
                ssize_t n = write(conn->fd, msg.data(), msg.size());
                (void)n;
                written_count.fetch_add(1, std::memory_order_relaxed);
                did_work = true;
            }
        }

        if (!did_work) {
            if (!running.load(std::memory_order_relaxed)) break;
            std::this_thread::sleep_for(std::chrono::microseconds(200));
        }
    }
}

int main() {
    constexpr int PORT = 18889;
    constexpr int NUM_WORKERS = 4;
    constexpr int NUM_TEST_CLIENTS = 20;

    std::atomic<bool> running{true};
    std::atomic<long long> processed{0}, written{0};

    std::vector<SPSCQueue<Task, 1024>> worker_input_storage(NUM_WORKERS);
    std::vector<SPSCQueue<Task, 1024>*> worker_input_ptrs;
    for (auto& q : worker_input_storage) worker_input_ptrs.push_back(&q);

    std::unordered_map<int, std::shared_ptr<ClientConnection>> clients;
    std::mutex clients_mutex;

    EpollReactor reactor(PORT, worker_input_ptrs, running);
    std::thread reactor_thread([&] { reactor.run(); });

    std::vector<std::thread> workers;
    for (int w = 0; w < NUM_WORKERS; ++w) {
        workers.emplace_back(worker_loop, w, std::ref(*worker_input_ptrs[w]),
                              std::ref(clients), std::ref(clients_mutex),
                              std::ref(running), std::ref(processed));
    }

    std::thread writer_thread(writer_loop, std::ref(clients), std::ref(clients_mutex),
                               std::ref(running), std::ref(written));

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::vector<std::thread> test_clients;
    std::atomic<int> success_count{0};
    for (int c = 0; c < NUM_TEST_CLIENTS; ++c) {
        test_clients.emplace_back([&, c] {
            int sock = socket(AF_INET, SOCK_STREAM, 0);
            sockaddr_in addr{};
            addr.sin_family = AF_INET;
            addr.sin_port = htons(PORT);
            inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
            if (connect(sock, (sockaddr*)&addr, sizeof(addr)) < 0) { close(sock); return; }
            std::string msg = "hello from client " + std::to_string(c);
            ssize_t w = write(sock, msg.data(), msg.size()); (void)w;
            char buf[256] = {};
            ssize_t n = read(sock, buf, sizeof(buf) - 1);
            if (n > 0) success_count.fetch_add(1, std::memory_order_relaxed);
            close(sock);
        });
    }
    for (auto& t : test_clients) t.join();

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    running.store(false, std::memory_order_relaxed);

    reactor_thread.join();
    for (auto& t : workers) t.join();
    writer_thread.join();

    std::cout << "Успешных клиентов: " << success_count.load() << " / " << NUM_TEST_CLIENTS << "\n";
    std::cout << "Обработано воркерами: " << processed.load() << "\n";
    std::cout << "Записано writer'ом: " << written.load() << "\n";

    return 0;
}
```

```
g++ -std=c++20 -O2 -pthread spsc_writer_demo.cpp -o spsc_writer_demo
./spsc_writer_demo
```

### Ключевая архитектурная идея — sharding вместо MPSC

**Sharding по `fd % num_workers`** — это то, что превращает потенциальный MPSC (несколько воркеров могли бы писать одному клиенту) в честный SPSC: конкретный `fd` **навсегда** закреплён за одним воркером на всё время жизни соединения, поэтому только этот воркер когда-либо кладёт сообщения в `outbox` этого клиента. Как следствие, `SPSCQueue` — без единого CAS вообще, что дешевле любой MPSC/MPMC-структуры, разобранных раньше.

**Три уровня SPSC в этой архитектуре:**

1. Реактор → воркер: реактор один (единственный producer), воркер один (единственный consumer своей входной очереди).
2. Воркер → per-client outbox: конкретный воркер один (гарантировано sharding'ом), writer один.
3. **Writer — единственный на всё приложение**, кто когда-либо вызывает `write()` на сокет — это убирает гонку из предыдущей версии, где два воркера потенциально могли бы писать в один и тот же fd одновременно и порвать данные в сокете.

### Компромисс, который стоит понимать

**Sharding по `fd % N`** равномерно распределяет соединения при условии, что fd выделяются достаточно равномерно (обычно так и есть в Linux). Но если один "тяжёлый" клиент шлёт сообщения намного чаще остальных на том же воркере — этот воркер станет bottleneck'ом, а остальные будут простаивать (классический hot partition в шардированных системах). Для реального прода нужен либо hash по client ID с учётом load balancing, либо work-stealing между воркерами (что снова вернуло бы вопрос MPSC/MPMC).

**Единственный writer-поток** — тоже упрощение для наглядности: под большой нагрузкой (десятки тысяч соединений) один поток может не успевать проходить по всем outbox'ам достаточно часто. В продакшене обычно либо несколько writer-потоков (каждый со своим подмножеством клиентов через тот же sharding-принцип), либо интеграция с `EPOLLOUT` — писать не в busy-loop, а по сигналу "сокет готов принять данные".

Готовы перейти к **broadcast** (следующий пункт плана — copy-on-write список подписчиков + RCU-style рассылка), или сначала добавить в этот код backpressure для outbox (сейчас просто "drop и лог в cerr")?

---
---

### **Broadcast**: тут MPSC/MPMC не всегда лучший выбор — часто эффективнее copy-on-write список подписчиков (RCU-style) + каждому подписчику своя SPSC-очередь, чем один общий lock-free broadcast-канал


Реализую broadcast-канал на copy-on-write списке подписчиков (RCU-style read) — каждый подписчик получает сообщение через свою SPSC-очередь, без единого общего lock-free канала.Good — процесс не завис. Причина зависания найдена: bounded SPSC-очередь (capacity 1024) переполняется под 80 000 сообщений, `publish()` тихо дропает лишние, а consumer ждёт получить ровно `TOTAL_MESSAGES` — которого никогда не наберётся. Исправляю: `publish()` делает retry вместо тихого дропа (backpressure, как в предыдущих демках), плюс отдельно уберу лишний `|| true`.Важный момент, который стоит проговорить явно: в моём первом варианте теста 4 publisher-потока **одновременно** писали в один и тот же `inbox` подписчика — а `SPSCQueue` рассчитан ровно на одного producer'а. С несколькими параллельными publisher'ами это уже реальная гонка (multiple producers на "single producer" структуру), а не гипотетическая. Исправляю тест на архитектурно корректный: **один поток владеет топиком** и публикует последовательно — это и есть условие, при котором `SPSC` на подписчика вообще имеет право быть SPSC.Теперь пересоберу с таймаутом на выполнение, чтобы избежать повторного зависания при наличии остаточного бага.Нашёл настоящую причину зависания — это важный практический урок, а не просто баг демки. Поправлю архитектурно правильно.Оба теста прошли: тест 1 — точная доставка 80 000/80 000 всем 5 подписчикам, тест 2 — 165 201 публикация под непрерывным churn (216 725 subscribe/unsubscribe) без единого зависания. Один drop в логе — ровно ожидаемое поведение bounded-retry policy на пограничном случае. Проверю на TSan:TSan чист. Теперь уберу отладочный маркер и покажу финальный код.Итоговый рабочий код — прошёл функциональный тест (80 000/80 000 доставлено) и TSan без единого предупреждения:

```cpp
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>
#include <memory>
#include <chrono>
#include <new>

// ============================================================
// SPSC ring buffer -- один producer, один consumer на подписчика
// ============================================================
template<typename T, size_t Capacity>
class SPSCQueue {
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be power of 2");
    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> write_pos_{0};
    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> read_pos_{0};
    T buffer_[Capacity];
public:
    bool push(T value) {
        size_t w = write_pos_.load(std::memory_order_relaxed);
        size_t r = read_pos_.load(std::memory_order_acquire);
        if (w - r >= Capacity) return false;
        buffer_[w & (Capacity - 1)] = std::move(value);
        write_pos_.store(w + 1, std::memory_order_release);
        return true;
    }
    bool pop(T& result) {
        size_t r = read_pos_.load(std::memory_order_relaxed);
        size_t w = write_pos_.load(std::memory_order_acquire);
        if (r == w) return false;
        result = std::move(buffer_[r & (Capacity - 1)]);
        read_pos_.store(r + 1, std::memory_order_release);
        return true;
    }
};

struct Subscriber {
    int id;
    SPSCQueue<std::string, 1024> inbox;
    std::atomic<long long> received{0};
};

// ============================================================
// BroadcastChannel: copy-on-write список подписчиков (RCU-style)
// ============================================================
class BroadcastChannel {
public:
    using SubscriberList = std::vector<std::shared_ptr<Subscriber>>;

private:
    std::atomic<std::shared_ptr<const SubscriberList>> subscribers_;

public:
    BroadcastChannel() {
        subscribers_.store(std::make_shared<const SubscriberList>());
    }

    void subscribe(std::shared_ptr<Subscriber> sub) {
        std::shared_ptr<const SubscriberList> old_list, new_list;
        do {
            old_list = subscribers_.load(std::memory_order_acquire);
            auto copy = std::make_shared<SubscriberList>(*old_list);
            copy->push_back(sub);
            new_list = std::move(copy);
        } while (!subscribers_.compare_exchange_weak(
                     old_list, new_list,
                     std::memory_order_acq_rel, std::memory_order_acquire));
    }

    void unsubscribe(int subscriber_id) {
        std::shared_ptr<const SubscriberList> old_list, new_list;
        do {
            old_list = subscribers_.load(std::memory_order_acquire);
            auto copy = std::make_shared<SubscriberList>();
            copy->reserve(old_list->size());
            for (auto& s : *old_list) {
                if (s->id != subscriber_id) copy->push_back(s);
            }
            new_list = std::move(copy);
        } while (!subscribers_.compare_exchange_weak(
                     old_list, new_list,
                     std::memory_order_acq_rel, std::memory_order_acquire));
    }

    // ВАЖНО: если подписчик отписался, но publisher уже держит
    // старый snapshot, он всё ещё видит этот shared_ptr<Subscriber>.
    // Без границы retry зависший/отключённый подписчик заблокировал
    // бы доставку ВСЕМ остальным -- отсюда MAX_RETRY + drop.
    void publish(const std::string& message) {
        std::shared_ptr<const SubscriberList> snapshot =
            subscribers_.load(std::memory_order_acquire);

        constexpr int MAX_RETRY = 1000;
        for (const auto& sub : *snapshot) {
            int attempts = 0;
            while (!sub->inbox.push(message)) {
                if (++attempts >= MAX_RETRY) {
                    std::cerr << "[broadcast] subscriber " << sub->id
                              << " not draining, dropping message\n";
                    break;
                }
                std::this_thread::yield();
            }
        }
    }
};
```

## Ключевая идея — почему COW + per-subscriber SPSC лучше общего broadcast-канала

**Общий MPMC/MPSC broadcast-канал** означал бы: все подписчики читают из **одной** структуры, и там нужна была бы либо фильтрация "это сообщение не моё" на каждого читателя, либо сложная схема с reference counting на каждое сообщение (сколько подписчиков ещё не прочитали — когда все прочитали, можно освобождать). Это создаёт **contention между самими подписчиками** за общую структуру — ровно то, чего лучше избегать.

**COW-список + persubscriber SPSC** разносит это на два независимых механизма:

- `publish()` — просто **один `atomic load`** shared_ptr'а (снимок списка), затем независимая работа с каждым SPSC — **никакого contention между publish-вызовами и между подписчиками друг с другом**.
- `subscribe`/`unsubscribe` — редкие операции, единственные, что платят цену (копирование всего вектора + CAS-retry), что приемлемо, потому что подключение/отключение случается на порядки реже, чем broadcast сообщений.

## RCU-семантика — "eventually consistent" членство

Snapshot, который держит `publish()`, может быть на долю секунды устаревшим относительно только что вызванного `subscribe()`/`unsubscribe()` — это **осознанный компромисс**, а не баг: новый подписчик может не получить одно-два сообщения, отправленных прямо в момент его подключения, а отписавшийся может получить одно лишнее после `unsubscribe()`. Для broadcast-паттерна (presence, статусы, чат-сообщения с идемпотентной доставкой) это обычно приемлемо; там, где нужна строгая консистентность членства, RCU-подход не годится.

## Реальный баг, который я поймал при тестировании (стоит знать заранее)

Первая версия `publish()` с **бесконечным retry** зависала намертво: если подписчик отписывался, пока publisher ещё держал старый snapshot с ссылкой на него, publisher вечно пытался писать в очередь, которую больше никто не читает — **и блокировал доставку всем остальным подписчикам** (publish идёт последовательно по списку). `MAX_RETRY` + drop с логом — обязательная защита, не опциональная оптимизация, для любой системы, где consumer может исчезнуть, пока producer держит на него ссылку.

**Сборка и тесты:**

```bash
g++ -std=c++20 -O2 -pthread broadcast_cow.cpp -o broadcast_cow -latomic
./broadcast_cow

g++ -std=c++20 -O0 -g -fsanitize=thread -pthread broadcast_cow.cpp -o broadcast_cow_tsan -latomic
./broadcast_cow_tsan
```

Готовы перейти к **Backpressure** (следующий пункт Блока 3 — уже частично затронут через `MAX_RETRY`, но стоит формализовать policy: drop oldest vs drop newest vs disconnect), или к **Reclamation problem** (что происходит с памятью подписчика при `unsubscribe`, пока publisher ещё держит на него shared_ptr — мы полагались на refcount, но стоит явно разобрать, чем это отличается от hazard pointers для этого конкретного случая)?


---
---

### **Backpressure**: что делать, если consumer медленнее producer — bounded queue + drop policy vs unbounded + memory blowup

Вижу два момента для исправления: расчёт MB неверный (лишнее деление), и DropOldest не считает реально потерянные сообщения (они дропаются молча внутри `push`, но метод возвращает `true`). Поправлю для честности данных.Теперь метрики честные. За 1.5 секунды unbounded-очередь выросла до **2 ГБ памяти** и продолжила бы расти — реальный OOM-сценарий при медленном клиенте и долгой сессии.

## Разбор результатов

**DisconnectConsumer** — самая показательная policy: `pushed=64` (ровно capacity), затем **немедленно** обнаружена проблема и producer остановлен — вместо бесконечных попыток впихнуть данные не успевающему клиенту, система явно признаёт "этот consumer не справляется" и прекращает работу с ним за один цикл переполнения.

**DropOldest** — `pushed=2 966 657`, но `overwritten=2 966 496` — то есть из почти 3 миллионов "успешных" push реально сохранилось только 64 (то, что осталось в очереди на момент остановки) плюс 97 уже потреблённых. Формально `push()` возвращал `true` почти всегда, но **99.98% сообщений были молча выброшены** сразу после записи — это архитектурно нормально для presence-обновлений ("печатает…", координаты курсора), где важно только последнее состояние, но было бы катастрофой для истории сообщений чата.

**DropNewest** — `dropped=3 534 030` явных потерь, `pushed=160` реально попало в очередь — новые сообщения теряются, пока в очереди остаётся место для более старых, что подходит, если порядок/полнота истории важнее полноты потока.

## Код (ключевые части)

```cpp
enum class DropPolicy { BlockRetry, DropNewest, DropOldest, DisconnectConsumer };

bool push(std::string v) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (data_.size() < Capacity) {
        data_.push_back(std::move(v));
        return true;
    }
    switch (policy_) {
        case DropPolicy::DropNewest:
            return false; // новое теряется, старое остаётся

        case DropPolicy::DropOldest:
            data_.pop_front();
            data_.push_back(std::move(v));
            overwritten_.fetch_add(1, std::memory_order_relaxed);
            return true; // новое принято, старое реально потеряно

        case DropPolicy::DisconnectConsumer:
            consumer_disconnected_.store(true, std::memory_order_release);
            return false; // явный сигнал наверх: клиент не успевает, отключить

        case DropPolicy::BlockRetry:
            return false; // обрабатывается в push_blocking снаружи
    }
    return false;
}

// Для BlockRetry: producer явно ждёт с таймаутом, а не молча теряет данные
bool push_blocking(std::string v, std::chrono::milliseconds timeout) {
    auto deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline) {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            if (data_.size() < Capacity) {
                data_.push_back(std::move(v));
                return true;
            }
        }
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    return false; // не дождались -- вызывающий код сам решает: разорвать соединение, залогировать, и т.д.
}
```

## Как выбирать policy для реального чата

|Тип данных|Рекомендуемая policy|Почему|
|---|---|---|
|Сообщения чата (текст, файлы)|**DisconnectConsumer** или `BlockRetry` с коротким таймаутом|Потеря сообщения — баг с точки зрения пользователя; лучше явно разорвать соединение и заставить клиента переподключиться с полной синхронизацией, чем тихо терять часть истории|
|Presence / "печатает…" / online-статус|**DropOldest**|Важно только последнее состояние; старое устарело в момент, когда пришло новое|
|Метрики / телеметрия|**DropNewest**|Не критично потерять недавние точки при всплеске нагрузки, важнее не тратить ресурсы на бесконечный retry|
|Критичные системные события (модерация, бан)|`BlockRetry` с достаточным таймаутом, либо отдельный **приоритетный** канал с большей capacity|Эти события не должны теряться вообще — иногда стоит завести для них отдельную структуру с более щедрым бюджетом памяти|

## Связь с MAX_RETRY из broadcast-демо

`MAX_RETRY` + drop в предыдущем `BroadcastChannel::publish()` — это фактически гибрид `BlockRetry` (ограниченный по числу попыток, не по времени) и `DropNewest` (после исчерпания попыток теряем именно новое сообщение для конкретного зависшего подписчика, а не всю рассылку). В реальном проде эту policy стоит параметризовать так же, как здесь — разным типам broadcast-сообщений (обычный текст vs presence) нужна разная стратегия.

**Полный код** (`backpressure_demo.cpp`) компилируется и запускается так же, как предыдущие демки:

```bash
g++ -std=c++20 -O2 -pthread backpressure_demo.cpp -o backpressure_demo
./backpressure_demo
```

Готовы перейти к последнему пункту Блока 3 — **reclamation problem** для broadcast-подписчиков (что происходит с памятью при disconnect, пока publisher ещё держит snapshot), или собрать всё в единый прототип чат-шины, как изначально планировали?

---
---

### **Reclamation проблема**: если пользователь отключается, когда безопасно освободить его структуры при lock-free доступе (hazard pointers / epoch-based / RCU)

Это ровно та reclamation-проблема, которую мы уже неявно решили в `broadcast_cow.cpp` через `shared_ptr` — но стоит явно показать **почему** это работает и **какой ценой**, а не просто "работает и ладно".Именно то, что нужно было доказать: `unsubscribe(1)` выполнился **между** взятием snapshot'а и его использованием, но "УДАЛЁН" для Subscriber 1 появляется **только после** строки "отправляю... подписчику 1" — то есть публикация безопасно отработала с уже "удалённым" (с точки зрения канала) подписчиком, и реальное освобождение памяти произошло ровно в момент выхода `snapshot` из scope, не раньше.

## Разбор — почему это работает и это НЕ hazard pointers, НЕ epoch-based

Каждый `shared_ptr<Subscriber>` внутри `SubscriberList` — это независимый refcount. Когда `publish()` берёт `snapshot = subscribers_.load()`, он получает `shared_ptr<const SubscriberList>` — refcount инкрементится **один раз** для всего вектора, а не по разу на каждого подписчика (вектор просто хранит копии `shared_ptr<Subscriber>`, которые были скопированы туда во время `subscribe()`/`unsubscribe()`, а не во время `publish()`).

Когда `unsubscribe(1)` строит новый вектор, он **не трогает** старый — старый вектор (тот, что держит `snapshot`) продолжает жить, пока у него есть хоть одна ссылка. `sub1`'s refcount остаётся ≥ 1, потому что старый вектор (внутри snapshot publisher'а) всё ещё хранит на него `shared_ptr`. Только когда `snapshot` разрушается на выходе из `publish_slow()` — refcount вектора падает до нуля → вектор разрушается → refcount каждого `shared_ptr<Subscriber>` внутри падает → если это была последняя ссылка на конкретного Subscriber, **тогда** вызывается его деструктор.

## Сравнение с hazard pointers и epoch-based — в чём принципиальная разница

||shared_ptr refcounting (наш случай)|Hazard Pointers|Epoch-based (RCU)|
|---|---|---|---|
|**Что отслеживается**|Число активных ссылок на **конкретный объект**|Явный список "кто сейчас держит какой указатель"|Глобальный счётчик "поколений", в какой эпохе кто находится|
|**Когда освобождается**|Точно в момент, когда счётчик достиг нуля — сразу, без задержки|Сразу, как только объект больше ни в одном hazard-слоте|Пакетно — когда все потоки "продвинулись" за эпоху удаления|
|**Стоимость на чтение**|Atomic inc/dec refcount при каждом **копировании** shared_ptr (здесь — не на каждый publish, а только при создании нового snapshot внутри subscribe/unsubscribe)|Atomic store hazard-слота + периодическая проверка scan|Atomic load эпохи при входе в критическую секцию|
|**Нужен ли явный "protected" список**|Нет — сам механизм refcounting уже это даёт|Да, отдельная инфраструктура|Да, отдельная инфраструктура|

**Ключевое отличие: `shared_ptr` даёт reclamation "из коробки" за счёт того, что каждый объект уже несёт свой счётчик** — не нужно городить отдельный per-thread hazard-реестр или эпохи, как в Treiber stack на raw pointers из более ранних разборов. Цена — тот же overhead, что разбирали для `atomic<shared_ptr<T>>`: atomic refcount operations при каждом копировании `shared_ptr`, и потенциальная нелок-фри реализация внутри самого `atomic<shared_ptr<...>>` на некоторых компиляторах (стоит проверить `is_lock_free()`, как обсуждали).

## Почему для broadcast-канала это оправданный выбор, а не для Treiber stack

**Частота операций разная.** В Treiber stack push/pop — это _hot path_, тысячи операций в секунду на структуру с миллионами узлов, там refcount-overhead на _каждую_ операцию заметен. В broadcast-канале refcount-инкремент происходит только при `subscribe`/`unsubscribe` (создание нового вектора с копиями shared_ptr) — **редко**, а сам `publish()` лишь читает уже существующие `shared_ptr` по ссылке (`const auto& sub`), не копируя и не трогая refcount вообще. Именно поэтому в комментариях к коду это было отмечено как "оправданное использование" — переиспользование дорогого механизма именно там, где он почти ничего не стоит.

## Практический вывод для собеседования

Если спросят "как вы решаете reclamation problem для списка подписчиков при disconnect" — правильный ответ не "используем hazard pointers всегда", а: **выбор механизма reclamation зависит от частоты read/write операций конкретной структуры.** Для hot-path lock-free структур (очереди, стеки с миллионами операций/сек) — hazard pointers или epoch-based, потому что там refcount-overhead был бы недопустим. Для структур с редкими мутациями и частыми, но дешёвыми чтениями (списки подписчиков, конфигурация, roster) — `atomic<shared_ptr<T>>` через COW часто оптимальнее по соотношению "простота кода / производительность", и вы уже получили reclamation бесплатно, без отдельной инфраструктуры.

Это закрывает последний пункт вашего плана Блока 3. Мы прошли весь путь от memory_order до полного прототипа чат-сервера (epoll-реактор → sharded воркеры → per-client SPSC → COW broadcast → backpressure → reclamation). Хотите собрать финальную сводку по всем трём блокам в виде документа/шпаргалки для подготовки к собеседованию, или есть конкретные темы, которые стоит повторить отдельно?

---
---

### **Практика:** собрать минимальный прототип — N потоков-писателей кладут сообщения в шину, M потоков-читателей (эмулируют коннекты) их разбирают, с метриками latency/throughput под нагрузкой.

Финальная практика — соберу самостоятельный прототип: N писателей кладут в общую bounded MPMC-шину, M читателей её разбирают, с полными метриками latency (p50/p99/p999) и throughput под нагрузкой.Все три сценария прошли без потерь (`dropped=0` везде), 8-9M msg/sec throughput, latency в диапазоне сотен микросекунд. Проверю на TSan для полной уверенности:TSan чист — все три сценария прошли без единого предупреждения о гонках. Вот полный код:

```cpp
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <algorithm>
#include <new>
#include <cassert>
#include <iomanip>

// ============================================================
// MPMC bus (Vyukov) -- итоговая версия из всех разборов:
// padding против false sharing, bounded с явным backpressure.
// ============================================================
template<typename T>
class MPMCBus {
    struct Cell { std::atomic<size_t> sequence; T data; };

    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> enqueue_pos_;
    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> dequeue_pos_;
    Cell* buffer_;
    size_t buffer_mask_;

public:
    explicit MPMCBus(size_t capacity)
        : buffer_(new Cell[capacity]), buffer_mask_(capacity - 1) {
        assert((capacity >= 2) && ((capacity & (capacity - 1)) == 0));
        for (size_t i = 0; i < capacity; ++i)
            buffer_[i].sequence.store(i, std::memory_order_relaxed);
        enqueue_pos_.store(0, std::memory_order_relaxed);
        dequeue_pos_.store(0, std::memory_order_relaxed);
    }
    ~MPMCBus() { delete[] buffer_; }
    MPMCBus(const MPMCBus&) = delete;

    bool try_push(T value) {
        Cell* cell; size_t pos = enqueue_pos_.load(std::memory_order_relaxed);
        for (;;) {
            cell = &buffer_[pos & buffer_mask_];
            size_t seq = cell->sequence.load(std::memory_order_acquire);
            intptr_t dif = (intptr_t)seq - (intptr_t)pos;
            if (dif == 0) {
                if (enqueue_pos_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) break;
            } else if (dif < 0) return false; // шина полна -- backpressure наверх
            else pos = enqueue_pos_.load(std::memory_order_relaxed);
        }
        cell->data = std::move(value);
        cell->sequence.store(pos + 1, std::memory_order_release);
        return true;
    }

    bool try_pop(T& result) {
        Cell* cell; size_t pos = dequeue_pos_.load(std::memory_order_relaxed);
        for (;;) {
            cell = &buffer_[pos & buffer_mask_];
            size_t seq = cell->sequence.load(std::memory_order_acquire);
            intptr_t dif = (intptr_t)seq - (intptr_t)(pos + 1);
            if (dif == 0) {
                if (dequeue_pos_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) break;
            } else if (dif < 0) return false; // шина пуста
            else pos = dequeue_pos_.load(std::memory_order_relaxed);
        }
        result = std::move(cell->data);
        cell->sequence.store(pos + buffer_mask_ + 1, std::memory_order_release);
        return true;
    }
};

// Сообщение несёт временную метку отправки -- нужна для честного
// измерения latency: время от push() ДО реального pop() (end-to-end).
struct Message {
    int producer_id;
    long long seq;
    std::chrono::steady_clock::time_point sent_at;
};

struct Metrics {
    std::vector<long long> latencies_ns;
    long long processed = 0;
};

void print_percentiles(const char* label, std::vector<long long>& lat) {
    if (lat.empty()) { std::cout << label << ": нет данных\n"; return; }
    std::sort(lat.begin(), lat.end());
    auto pct = [&](double p) {
        size_t idx = static_cast<size_t>(p * (lat.size() - 1));
        return lat[idx];
    };
    std::cout << label << " (n=" << lat.size() << "):\n"
              << "  p50:  " << std::setw(8) << pct(0.50) << " ns\n"
              << "  p90:  " << std::setw(8) << pct(0.90) << " ns\n"
              << "  p99:  " << std::setw(8) << pct(0.99) << " ns\n"
              << "  p999: " << std::setw(8) << pct(0.999) << " ns\n"
              << "  max:  " << std::setw(8) << lat.back() << " ns\n";
}

struct PrototypeResult {
    long long total_produced = 0;
    long long total_consumed = 0;
    long long total_dropped = 0;
    double wall_time_sec = 0;
    std::vector<long long> all_latencies_ns;
};

PrototypeResult run_prototype(int num_writers, int num_readers,
                                int messages_per_writer, size_t bus_capacity) {
    MPMCBus<Message> bus(bus_capacity);
    const long long total_target = static_cast<long long>(num_writers) * messages_per_writer;

    std::atomic<long long> produced{0}, consumed{0}, dropped{0};

    std::vector<Metrics> reader_metrics(num_readers);
    for (auto& m : reader_metrics) m.latencies_ns.reserve(messages_per_writer * num_writers / num_readers + 16);

    auto start = std::chrono::steady_clock::now();

    std::vector<std::thread> writers;
    for (int w = 0; w < num_writers; ++w) {
        writers.emplace_back([&, w] {
            for (int i = 0; i < messages_per_writer; ++i) {
                Message msg{w, i, std::chrono::steady_clock::now()};
                int attempts = 0;
                constexpr int MAX_ATTEMPTS = 10000;
                while (!bus.try_push(msg)) {
                    if (++attempts >= MAX_ATTEMPTS) {
                        dropped.fetch_add(1, std::memory_order_relaxed);
                        goto next_message;
                    }
                    std::this_thread::yield();
                }
                produced.fetch_add(1, std::memory_order_relaxed);
                next_message:;
            }
        });
    }

    std::vector<std::thread> readers;
    for (int r = 0; r < num_readers; ++r) {
        readers.emplace_back([&, r] {
            Message msg;
            auto& metrics = reader_metrics[r];
            while (consumed.load(std::memory_order_relaxed) + dropped.load(std::memory_order_relaxed) < total_target) {
                if (bus.try_pop(msg)) {
                    auto now = std::chrono::steady_clock::now();
                    auto latency_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now - msg.sent_at).count();
                    metrics.latencies_ns.push_back(latency_ns);
                    metrics.processed++;
                    consumed.fetch_add(1, std::memory_order_relaxed);
                } else {
                    std::this_thread::yield();
                }
            }
        });
    }

    for (auto& t : writers) t.join();
    for (auto& t : readers) t.join();

    auto end = std::chrono::steady_clock::now();

    PrototypeResult result;
    result.total_produced = produced.load();
    result.total_consumed = consumed.load();
    result.total_dropped = dropped.load();
    result.wall_time_sec = std::chrono::duration<double>(end - start).count();
    for (auto& m : reader_metrics) {
        result.all_latencies_ns.insert(result.all_latencies_ns.end(),
                                        m.latencies_ns.begin(), m.latencies_ns.end());
    }
    return result;
}

void print_result(const char* label, const PrototypeResult& r) {
    std::cout << "\n===== " << label << " =====\n";
    std::cout << "produced=" << r.total_produced << ", consumed=" << r.total_consumed
              << ", dropped=" << r.total_dropped << "\n";
    std::cout << "wall time: " << r.wall_time_sec << " s\n";
    std::cout << "throughput: " << static_cast<long long>(r.total_consumed / r.wall_time_sec) << " msg/sec\n";
    auto lat_copy = r.all_latencies_ns;
    print_percentiles("end-to-end latency", lat_copy);
}

int main() {
    constexpr size_t BUS_CAPACITY = 4096;
    constexpr int MESSAGES_PER_WRITER = 100'000;

    auto r1 = run_prototype(4, 4, MESSAGES_PER_WRITER, BUS_CAPACITY);
    print_result("4 writers / 4 readers", r1);

    auto r2 = run_prototype(8, 2, MESSAGES_PER_WRITER / 2, BUS_CAPACITY);
    print_result("8 writers / 2 readers (reader-bound)", r2);

    auto r3 = run_prototype(2, 8, MESSAGES_PER_WRITER, BUS_CAPACITY);
    print_result("2 writers / 8 readers (writer-bound)", r3);

    return 0;
}
```

**Сборка и запуск:**

```bash
g++ -std=c++20 -O2 -pthread final_prototype.cpp -o final_prototype
./final_prototype

# Обязательная проверка на гонки (уменьшите MESSAGES_PER_WRITER для скорости под TSan):
g++ -std=c++20 -O0 -g -fsanitize=thread -pthread final_prototype.cpp -o final_prototype_tsan
./final_prototype_tsan
```

## Что показывают три сценария

**4/4 симметричный** — baseline: 8.1M msg/sec, p50 245 мкс. p999 (833 мкс) заметно выше p50 — типичный длинный хвост из-за yield-based busy-wait вместо более умного backoff.

**8 writers / 2 readers (reader-bound)** — читатели становятся узким местом (2 потока разбирают вдвое больший поток писателей), но throughput **не упал**, а даже чуть вырос (9.3M msg/sec) — потому что per-slot contention MPMC-очереди хорошо масштабируется на запись даже при малом числе читателей. p99 здесь лучше (350 мкс vs 683 мкс) — меньше читателей означает меньше конкуренции за `dequeue_pos_`.

**2 writers / 8 readers (writer-bound)** — читатели простаивают (yield-loop), пока писателей мало — самая низкая latency на всех перцентилях (p999 всего 356 мкс) и самый чистый хвост, потому что как только сообщение появляется, кто-то из 8 читателей почти сразу его подхватывает.

## Важная методологическая оговорка

Этот контейнер даёт **1 видимый CPU** — реальной параллельности между потоками физически нет, только чередование планировщиком. Абсолютные цифры throughput здесь **не показательны** для настоящей многоядерной машины (там ожидается на порядок выше throughput при реальной параллельности, но и заметно более выраженная разница между reader-bound/writer-bound сценариями из-за реальной cache-line contention между ядрами, а не только на одном ядре). Обязательно перегоните это на своей рабочей машине — особенно интересно посмотреть на `perf c2c` при 8/8 конфигурации, чтобы увидеть реальный false-sharing/contention паттерн на `enqueue_pos_`/`dequeue_pos_`.

Этим прототипом закрывается вся практика Блока 3, а вместе с ней и весь план — от memory model до полной архитектуры чат-сервера с метриками под нагрузкой.

---
---
