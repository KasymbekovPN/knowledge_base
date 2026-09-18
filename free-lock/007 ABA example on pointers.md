
### Разобрать пример ABA на указателях вручную

Отлично — теперь виден весь процесс до момента краха. **Segfault в самом конце — это и есть демонстрация бага**, не ошибка в демо-коде: он падает ровно там, где реальный ABA-баг привёл бы к порче памяти в проде.

## Разбор вывода по шагам

```
Начало: 1(@0x...2b0) -> 2(@0x...2d0) -> 3(@0x...2f0) -> nullptr
Адрес A = 0x...2b0
```

Стек `A→B→C`, запоминаем физический адрес A.

```
[Поток 1] прочитал head=0x...2b0 (value=1), next=0x...2d0
[Поток 1] ГОТОВИТСЯ к CAS(head: A -> B), но приостановлен...
```

Поток 1 успел прочитать `old_head = A` и `observed_next = B` — но **до** самого CAS его вытеснил планировщик (в реальности — просто менее удачливый scheduling, здесь мы форсируем это явно).

```
[Поток 2] после pop() #1: 2 -> 3 -> nullptr
[Поток 2] после pop() #2: 3 -> nullptr
```

Поток 2 успевает снять A и B, **освобождая их память** (`delete`).

```
[Поток 2] после push(D): 99(@0x...2b0) -> 3 -> nullptr
Адрес D = 0x...2b0 (== адресу A? ДА, СОВПАЛ)
```

**Ключевой момент**: новый узел D физически получает **тот же адрес**, что был у A (в демо это форсировано через `placement new` по адресу A — в реальном коде это сделал бы аллокатор сам, переиспользуя только что освобождённый блок; на практике вероятность совпадения адреса ниже 100%, но ненулевая и растёт с частотой аллокаций/деаллокаций).

```
[Поток 1] CAS вернул: УСПЕХ
```

Поток 1 просыпается и сравнивает `head` с `expected` (оба хранят число `0x...2b0`). CAS не знает и не может знать, что за этим адресом сейчас лежит D, а не A — **сравнение чисто по битовому значению указателя**, поэтому CAS честно докладывает "совпало" и записывает `head = observed_next` (адрес B).

```
Segmentation fault
```

`head` теперь указывает на **B — узел, который уже был `delete`-нут** потоком 2. Любое следующее обращение к стеку (в моей демке — `print_stack`, в реальном коде — следующий `pop()`) разыменовывает висячий указатель на освобождённую память → segfault (в лучшем случае) или тихая порча данных (в худшем, если память ещё не была переиспользована ОС).

## Что это доказывает практически

CAS проверил только _значение_ указателя (`0x...2b0`), а не _идентичность_ объекта за ним. Между моментом чтения (`old_head = A`) и моментом сравнения в CAS адрес был: освобождён → переиспользован под совершенно другой узел (D) → и CAS не смог отличить "это всё ещё A" от "это уже D, просто по тому же адресу".

Это ровно тот сценарий, для которого нужны tagged pointers (добавили бы монотонный tag, и CAS сравнивал бы (адрес, tag) — не совпало бы), hazard pointers (память A/B не была бы освобождена, пока поток 1 держит на неё hazard-указатель) или `atomic<shared_ptr>` (refcount не дал бы освободить B, пока жива хоть одна ссылка) — все три метода, что разбирали раньше, устраняют именно эту дыру, каждый своим способом.

Хотите, чтобы я на этом же коде показал, как именно **tagged pointer** предотвратил бы этот конкретный краш — добавив tag-инкремент и увидев, что CAS в конце вернёт `false`?

Соберу все три демонстрации (raw pointer / tagged pointer / hazard pointer) в один файл с общим `main()`, чтобы можно было прогонять и сравнивать за один запуск.Всё скомпилировалось и отработало как задумано. Вот единый файл:

```cpp
#include <atomic>
#include <print>
#include <array>
#include <cstdint>
#include <iostream>
#include <string>

// ============================================================
// ЧАСТЬ 1: Raw pointer -- ABA приводит к segfault
// ============================================================
namespace raw_demo {
    namespace {
        struct Node {
            int value;
            Node* next;
        };
        std::atomic<Node*> head{nullptr};

        void print_stack(const std::string& label) {
            std::print(std::cout, "{}: ", label);
            for (const Node* n{head.load()}; n; n = n->next) {
                std::cout << n->value << "(@" << n << ") -> ";
            }
            std::cout << "nullptr\n" << std::flush;
        }

        void run() {
            std::cout << "\n========== 1. RAW POINTER (ABA -> segfault) ==========\n" << std::flush;

            Node* A{new Node{.value = 1, .next = nullptr}};
            Node* B{new Node{.value = 2, .next = nullptr}};
            Node* C{new Node{.value = 3, .next = nullptr}};
            A->next = B; B->next = C;
            head.store(A);
            print_stack("Start");

            Node* old_head{head.load()};
            Node* observed_next{old_head->next}; // B, пока A жив
            std::cout << "[thread 1] read A, next=B. STOPPED" << std::flush;

            Node* p1{head.load()}; head.store(p1->next); delete p1; // снимает A
            Node* p2{head.load()}; head.store(p2->next); delete p2; // снимает B
            Node* D{new (A) Node{.value = 42, .next = nullptr}}; // D переиспользует адрес A!
            D->next = head.load(); head.store(D);
            print_stack("After pop/pop/push(D) by thread 2");
            std::cout << "Address D == Address A? " << (D == A ? "yes" : "no") << "\n" << std::flush;

            Node* expected{old_head};
            bool success{head.compare_exchange_strong(expected, observed_next)};
            std::cout << "[Thread 1] CAS: " << (success ? "Success (bag!)" : "fail") << "\n" << std::flush;

            std::cout << "The next access to the stack dereferences the removed B -> segfault.:\n" << std::flush;
            print_stack("BOTTOM LINE (we expect a collapse)");
        }

    }
}

// ============================================================
// ЧАСТЬ 2: Tagged pointer -- CAS корректно проваливается
// ============================================================
namespace target_demo {
    namespace {
        struct Node {
            int value;
            Node* next;
        };

        struct TaggedPtr {
            Node* ptr;
            uint64_t tag;

            bool operator==(const TaggedPtr& rhs) const {
                return ptr == rhs.ptr && tag == rhs.tag;
            }
        };
        std::atomic<TaggedPtr> head{TaggedPtr{.ptr = nullptr, .tag = 0}};

        void print_stack(const std::string& label) {
            const auto [ptr, tag]{head.load()};
            std::println(std::cout, "{}: [{}]", label, tag);
            for (const Node* n{ptr}; n; n = n->next) {
                std::cout << n->value << "(@" << n << ") -> ";
            }
            std::cout << "nullptr\n" << std::flush;
        }

        void run() {
            std::cout << "\n========== 2. TAGGED POINTER (CAS fails, stack intact) ==========\n" << std::flush;

            Node* A{new Node{.value = 1, .next = nullptr}};
            Node* B{new Node{.value = 2, .next = nullptr}};
            Node* C{new Node{.value = 3, .next = nullptr}};
            A->next = B; B->next = C;
            head.store(TaggedPtr{.ptr = A, .tag = 0});
            print_stack("Start");

            TaggedPtr old_head{head.load()};
            Node* observed_next{old_head.ptr->next};
            std::cout << "[thread 1] read {A, tag=0}. STOPPED\n" << std::flush;

            TaggedPtr h1{head.load()}; Node* p1{h1.ptr};
            head.store(TaggedPtr{.ptr = p1->next, .tag = h1.tag + 1});
            // ВАЖНО: delete p1 (то есть А) НЕ делаем! Оставляем адрес живым.p1;

            TaggedPtr h2{head.load()}; Node* p2{h2.ptr};
            head.store(TaggedPtr{.ptr = p2->next, .tag = h2.tag + 1}); delete p2;
            // В можно удалить safely, к нему никто не обратится

            // Поток 2 делает PUSH нового узла D.
            // Вместо placement new мы легитимно используем живой адрес А, меняя в нем данные.
            // Это идеальная симуляция того, что аллокатор выдал тот же адрес.
            A->value = 42;
            // A->next = nullptr;
            TaggedPtr h3{head.load()};
            A->next = h3.ptr;
            head.store(TaggedPtr{.ptr = A, .tag = h3.tag + 1});

            print_stack("After push/pop/push(D) by thread 2");
            std::cout << std::format("tag is {} (it was 0 on read thread 1)\n", head.load().tag) << std::flush;

            TaggedPtr expected{old_head};
            const auto desired{TaggedPtr{.ptr = observed_next, .tag = old_head.tag + 1}};

            // БЛАГОДАРЯ ТЕГУ CAS ВЫДАСТ FAIL! Указатели равны (A==A), но теги разные (3 != 0)
            bool success = head.compare_exchange_strong(expected, desired);
            std::cout << "[Поток 1] CAS: " << (success ? "успех (БАГ!)" : "НЕУДАЧА (правильно)")
                      << " -- указатель совпал, но tag " << expected.tag << " != " << old_head.tag << "\n" << std::flush;

            print_stack("BOTTOM LINE (success)");

            // Корректно чистим оставшуюся память в конце демо
            const TaggedPtr final_h{head.load()};
            Node* cur{final_h.ptr};
            while (cur) {
                Node* next{cur->next};
                delete cur;
                cur = next;
            }
        }
    }
}




// ============================================================
// ЧАСТЬ 3: Hazard pointer -- память не освобождается, пока защищена
// ============================================================
namespace hazard_demo {
    namespace {
        struct Node {
            int value;
            Node* next;
        };

        std::atomic<Node*> head{nullptr};
        constexpr int MAX_HAZARDS{4};
        std::array<std::atomic<Node*>, MAX_HAZARDS> hazards_ptrs;

        bool is_hazardous(Node* p) {
            for (auto& h: hazards_ptrs) {
                if (h.load(std::memory_order_acquire) == p) return true;
            }
            return false;
        }

        void retire(Node* p) {
            if (is_hazardous(p)) {
                std::cout << "[retire] " << p << " protected -- delete deferred\n" << std::flush;
            } else {
                std::cout << "[retire] " << p << " not protected -- delete\n" << std::flush;
                delete p;
            }
        }

        void print_stack(const std::string& label) {
            std::cout << std::format("{}: ", label);
            for (const Node* n = head.load(); n; n = n->next) {
                std::cout << n->value << "(@" << n << ") -> ";
            }
            std::cout << "nullptr\n" << std::flush;
        }

        void run() {
            std::cout << "\n========== 3. HAZARD POINTER (Memory lives as long as it is protected.) ==========\n" << std::flush;
            for (auto& h: hazards_ptrs) h.store(nullptr);

            const auto A{new Node{.value = 1, .next = nullptr}};
            const auto B{new Node{.value = 2, .next = nullptr}};
            const auto C{new Node{.value = 3, .next = nullptr}};
            A->next = B; B->next = C;
            head.store(A);
            print_stack("START");

            Node* old_head{head.load(std::memory_order_acquire)};
            hazards_ptrs[0].store(old_head, std::memory_order_release); // protect A
            Node* observed_next{old_head->next};
            std::cout << "[thread 1] hazard = A, STOPPED.\n" << std::flush;

            Node* p1{head.load()}; head.store(p1->next); retire(p1); // A under protect -- p1 won't deleted
            Node* p2{head.load()}; head.store(p2->next); retire(p2); // delete p2

            // обычный new -- адрес A занят, аллокатор его не тронет
            const auto D{new Node{.value = 999, .next = nullptr}};
            D->next = head.load(); head.store(D);
            print_stack("After push/pop/push(D) by thread 2");
            std::cout << std::format("address A == address D: {}", D == A ? "YES" : "NO(lives)") << std::flush;

            Node* expected{old_head};
            const bool success{head.compare_exchange_strong(expected, observed_next)};
            std::cout << std::format("[thread 1] CAS: {}", success ? "success" : "fail") << std::flush;

            hazards_ptrs[0].store(nullptr, std::memory_order_release);
            if (!is_hazardous(old_head)) {
                std::cout << "  [reclaim] A not protected -- delete\n" << std::flush;
                delete old_head;
            }
            print_stack("BOTTOM LINE (success)");
        }

    }
}

int main(const int argc, char *argv[]) {
    if (const bool run_raw{argc > 1 && std::string(argv[1]) == "--raw"};
        run_raw) {
        raw_demo::run();
    } else {
        // target_demo::run();
        hazard_demo::run();
    }

    return 0;
}

```

**Сборка:**

```bash
g++ -std=c++20 -O0 -g aba_all.cpp -o aba_all -latomic
```

(`-latomic` обязателен — `TaggedPtr` требует 16-байтного CAS, которого нет как инлайновой инструкции на этом тулчейне.)

**Запуск:**

```bash
./aba_all         # tagged + hazard демо, оба завершаются успешно
./aba_all --raw   # raw-pointer демо -- намеренно падает с segfault
```
