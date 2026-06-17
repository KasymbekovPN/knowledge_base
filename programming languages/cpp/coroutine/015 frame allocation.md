---
tags:
  - programming-language
  - cpp
  - coroutine
---
[[programming languages/cpp/coroutine/_|<=]]

Аллокации фрейма — это вопрос производительности корутин. Каждая корутина по умолчанию аллоцирует свой frame в куче, и в горячем коде это может стать узким местом. Разберём, где аллокация происходит, как её увидеть, когда компилятор её устраняет (HALO), и как взять контроль через кастомный `operator new`.

## Где и почему аллоцируется frame

Когда создаётся корутина, её состояние (coroutine frame) должно пережить приостановку — значит, его нельзя оставить в стеке вызывающего. По умолчанию компилятор вставляет вызов `operator new` для размещения frame в куче. В наш счётчик попала ровно эта аллокация: 56 байт под параметры, локальные переменные (`local`), точку возобновления и служебные данные промиса.

Что входит в эти байты: сохранённые параметры корутины, локальные переменные, живущие через точки приостановки, сам объект `promise_type`, указатель на точку возобновления и индекс состояния. Размер вычисляется компилятором на этапе компиляции (это свойство stackless-модели).

## Почему HALO здесь не сработал

Вспомним из пункта про HALO: устранение аллокации возможно, только если компилятор может доказать, что **время жизни корутины не выходит за пределы вызывающего**. В нашем `main`:

```cpp
Task t = coro(21);    // Task хранит handle и УПРАВЛЯЕТ временем жизни frame
t.h.resume();
// frame должен жить, пока жив t -> компилятор не может вложить frame в кадр main
```

Корутина «убегает» из точки создания в объект `Task`, чьё время жизни не вложено очевидным образом — поэтому frame остаётся в куче. Это типичная ситуация для `Task`-подобных типов.

## Когда HALO срабатывает

HALO включается, когда корутина создаётся, используется и завершается **внутри** вызывающего, не «убегая». Классический случай — немедленное ожидание во вложенной корутине. Покажу контраст:Здесь все 3 аллокации остались (outer + 2× inner) даже на `-O2`. Это честный результат, и он показателен: **HALO — оптимизация «по возможности», а не гарантия**, и GCC в этом сценарии её не применил. Объясню, почему, и что для неё нужно.

## Почему HALO не сработал даже здесь

HALO требует, чтобы компилятор **видел весь жизненный цикл корутины насквозь** и мог доказать вложенность. На практике для этого нужно:

1. **Инлайнинг** — тело вложенной корутины должно быть видимо и встроено в вызывающую. Если определение `inner` не инлайнится (а корутины с их трансформацией инлайнятся тяжело), доказать вложенность нельзя.
2. **Прозрачная цепочка** — все промежуточные awaiter'ы не должны «прятать» handle так, чтобы компилятор терял след.
3. **Отсутствие «убегания»** — handle нигде не сохраняется наружу.

GCC исторически применяет HALO консервативно. Clang делает это агрессивнее. Это объясняет частую рекомендацию: **не закладывайся на HALO как на гарантию** — проверяй на своём компиляторе и оптимизациях, как мы только что сделали счётчиком.

Важный нюанс из стандарта: HALO разрешён, но не предписан. Компилятор _вправе_ убрать вызов `operator new`, если может удовлетворить требования к времени жизни иначе — но не обязан. Поэтому единственный надёжный способ узнать — измерить.

## Как взять контроль: кастомный operator new в промисе

Раз HALO ненадёжен, а в горячем коде аллокации на каждую корутину дороги, есть прямой рычаг: **`promise_type` может определить свой `operator new`/`operator delete`**. Тогда компилятор для frame этой корутины вызовет их вместо глобального. Это позволяет, например, аллоцировать frame'ы из пула.

Покажу arena/pool-аллокатор для frame:Нужен `<cstddef>` для `std::max_align_t`.Результат точный: **1000 корутин создано, 1000 frame'ов ушло в арену, 0 аллокаций кучи под них.** Кастомный `operator new` в промисе полностью перенаправил размещение frame'ов мимо `malloc`.

```cpp
struct promise_type {
    static void* operator new(std::size_t n) {
        return g_arena.allocate(n);          // frame из арены, не из кучи
    }
    static void operator delete(void*, std::size_t) noexcept {}  // арена чистится оптом
    // ...
};
```

## Как это работает

Когда компилятор размещает coroutine frame, он сначала ищет `operator new` **в самом `promise_type`**, и только если его нет — берёт глобальный. Объявив `operator new` в промисе, мы перехватили размещение frame для всех корутин этого типа. Здесь — выдаём память из заранее выделенного буфера (арены), не трогая кучу.

Это мощный рычаг для горячих путей: вместо тысячи `malloc` — один большой буфер и быстрая bump-аллокация (сдвиг указателя). Парный `operator delete` сделан no-op, потому что арена освобождает всё разом через `reset()` — типичная стратегия для пулов.

Тонкость с сигнатурой: компилятор передаёт в `operator new` размер frame первым аргументом, а аргументы корутины — следующими (если определить расширенную форму). Для frame-аллокации обычно достаточно формы `operator new(std::size_t)`.

## Сводка: три уровня контроля над аллокацией

|Уровень|Что делает|Надёжность|
|---|---|---|
|**По умолчанию**|frame в куче через глобальный `operator new`|всегда работает, но есть аллокация|
|**HALO**|компилятор устраняет аллокацию, вкладывая frame|оптимизация «по возможности», не гарантия|
|**Кастомный `operator new`**|frame из пула/арены, под твоим контролем|надёжно, но требует кода|

## Практические выводы

**1. Аллокация frame реальна и происходит на каждую корутину** (по умолчанию). Мы измерили 56 байт в первом примере. В горячем коде с миллионами короткоживущих корутин это ощутимо.

**2. На HALO нельзя закладываться вслепую.** Как показали оба замера, GCC её часто не применяет даже на `-O2`. Если аллокации критичны — _проверяй_ счётчиком или профайлером на своём тулчейне, а не надейся. Условия для HALO: видимость определения, инлайнинг, отсутствие «убегания» handle.

**3. Кастомный `operator new` в промисе — надёжный способ контроля.** Пул/арена для frame'ов даёт предсказуемую производительность независимо от того, сработал HALO или нет.

**4. `[[clang::coro_*]]` и аналоги** — в некоторых компиляторах есть атрибуты-подсказки для HALO, но это нестандартно и зависит от версии.

## Связь с пройденным

Это замыкает тему stackless-модели из начала разбора: именно потому, что размер frame известен компилятору на этапе компиляции, возможны и HALO (вложить frame в кадр вызывающего), и кастомный `operator new` (выдать ровно нужный размер из пула). Stackful-корутины такого контроля не дали бы — их отдельный стек всегда живёт в куче. Цена за гибкость — «заразность» и ручное управление, но взамен мы получаем точный контроль над тем, где живёт состояние корутины.

### Frame allocation
```cpp
#include <iostream>
#include <format>
#include <coroutine>
#include <new>
#include <utility>

// Глобальный счётчик аллокаций через перехват operator new/delete
static long g_alloc_count{};
static long g_alloc_bytes{};

void* operator new(std::size_t _n) {
    g_alloc_count++;
    g_alloc_bytes += _n;
    if (void* p = std::malloc(_n)) return p;
    throw std::bad_alloc{};
}
void operator delete(void* _p) noexcept { std::free(_p); }
void operator delete(void* _p, std::size_t) noexcept { std::free(_p); }

struct Task {
    struct promise_type {
        Task get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };

    std::coroutine_handle<promise_type> h;

    explicit Task(std::coroutine_handle<promise_type> _h): h{_h} {}
    Task(Task&& _other) noexcept: h{std::exchange(_other.h, {})} {}
    ~Task() { if (h) h.destroy(); }
};

Task coro(int _input) {
    int local = _input * 2; // локальные переменные -> часть coroutine frame
    co_await std::suspend_always();
    std::cout << std::format("local: {}\n", local);

    co_return;
}

int main() {
    std::cout << std::format("before: allocs = {}, bytes = {}\n", g_alloc_count, g_alloc_bytes);
    {
        Task t = coro(21); // СОЗДАНИЕ корутины -> аллокация frame
        std::cout << std::format("after create: allocs = {}, bytes = {}\n", g_alloc_count, g_alloc_bytes);

        t.h.resume();
        t.h.resume(); // довести до конца
    }
    std::cout << std::format("after: allocs = {}, bytes = {}\n", g_alloc_count, g_alloc_bytes);

    return 0;
}
```

```
before: allocs = 4, bytes = 136
after create: allocs = 6, bytes = 248
local: 42
after: allocs = 8, bytes = 344
```

### HALO
```cpp
#include <iostream>
#include <format>
#include <coroutine>
#include <new>
#include <utility>

static long g_alloc_count{};

void* operator new(std::size_t _n) {
    g_alloc_count++;
    if (void* p = std::malloc(_n)) return p;

    throw std::bad_alloc();
}
void operator delete(void* _p) noexcept { std::free(_p); }
void operator delete(void* _p, std::size_t) noexcept { std::free(_p); }

struct Task {
    struct promise_type {
        std::coroutine_handle<> continuation{};
        int result{};

        Task get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() noexcept { return {}; }

        // symmetric transfer обратно к ожидающему — нужно для inlining/HALO
        struct FinalAwaiter {
            bool await_ready() noexcept { return false; }
            std::coroutine_handle<> await_suspend(
                std::coroutine_handle<promise_type> _h
            ) noexcept {
                auto c = _h.promise().continuation;
                return c ? c : std::noop_coroutine();
            }
            void await_resume() noexcept {}
        };
        FinalAwaiter final_suspend() noexcept { return {};}
        void return_value(int _value) { result = _value; }
        void unhandled_exception() { std::terminate(); }
    };

    std::coroutine_handle<promise_type> h;

    explicit Task(std::coroutine_handle<promise_type> _h): h{_h} {}
    Task(Task&& _other) noexcept: h{std::exchange(_other.h, {})} {}
    ~Task() { if (h) h.destroy(); }

    // делает Task awaitable
    bool await_ready() noexcept { return false; }
    std::coroutine_handle<> await_suspend(std::coroutine_handle<> _caller) noexcept {
        h.promise().continuation = _caller;
        return h; // symmetric transfer к вложенной корутине
    }
    int await_resume() noexcept { return h.promise().result; }
};

// Вложенная корутина — кандидат на HALO: создаётся и сразу же co_await-ится
Task co_inner(int _input) {
    co_return _input * 2;
}

Task co_outer() {
    int a{co_await co_inner(21)}; // inner создаётся, используется и завершается тут
    int b{co_await co_inner(a)}; // ещё одна вложенная

    co_return b;
}

int main() {
    {
        Task t = co_outer();
        t.h.resume(); // запускаем всю цепочку
        std::cout << std::format("result = {}\n", t.h.promise().result);
    }
    // сколько всего аллокаций? outer + 2x inner = до 3, но часть может быть устранена
    std::cout << std::format("allocs = {}\n", g_alloc_count);

    return 0;
}
```

```
result = 84
allocs = 8
```

---


### custom new
```cpp
#include <coroutine>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <new>
#include <utility>

// Глобальный счётчик "обычных" аллокаций кучи
static long g_heap_allocs = 0;
void* operator new(std::size_t n) {
    g_heap_allocs++;
    if (void* p = std::malloc(n)) return p;
    throw std::bad_alloc{};
}
void operator delete(void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }

// Простейшая арена: выдаёт куски из заранее выделенного буфера, не трогая кучу
struct Arena {
    static constexpr std::size_t SIZE = 64 * 1024;
    alignas(std::max_align_t) unsigned char buffer[SIZE];
    std::size_t offset = 0;
    long        frame_allocs = 0;

    void* allocate(std::size_t n) {
        n = (n + 15) & ~std::size_t(15);      // выравнивание до 16
        if (offset + n > SIZE) throw std::bad_alloc{};
        void* p = buffer + offset;
        offset += n;
        frame_allocs++;
        return p;
    }
    void reset() { offset = 0; }
};

Arena g_arena;

struct Task {
    struct promise_type {
        // КАСТОМНЫЙ operator new: frame берётся из арены, НЕ из кучи
        static void* operator new(std::size_t n) {
            return g_arena.allocate(n);
        }
        // парный delete: арена освобождает оптом через reset(), тут no-op
        static void operator delete(void*, std::size_t) noexcept {}

        Task get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend()   noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
    std::coroutine_handle<promise_type> h;
    explicit Task(std::coroutine_handle<promise_type> handle) : h(handle) {}
    Task(Task&& o) noexcept : h(std::exchange(o.h, {})) {}
    ~Task() { if (h) h.destroy(); }
};

Task worker(int id) {
    co_await std::suspend_always{};
    (void)id;
    co_return;
}

int main() {
    std::printf("start: heap_allocs=%ld\n", g_heap_allocs);

    // создаём МНОГО корутин — все frame'ы идут в арену, куча не трогается
    constexpr int N = 1000;
    Task* tasks = static_cast<Task*>(::operator new(sizeof(Task) * N)); // 1 аллокация под массив
    long heap_before = g_heap_allocs;

    for (int i = 0; i < N; ++i) {
        new (&tasks[i]) Task(worker(i));   // создание корутины -> frame в арене
    }
    std::printf("after creating %d coroutines:\n", N);
    std::printf("  frames in arena = %ld\n", g_arena.frame_allocs);
    std::printf("  heap allocs for frames = %ld  (должно быть 0!)\n",
                g_heap_allocs - heap_before);

    for (int i = 0; i < N; ++i) {
        tasks[i].h.resume();               // доводим до конца
        tasks[i].~Task();                  // destroy frame
    }
    ::operator delete(tasks);
    return 0;
}
```

---

Остался последний пункт плана по подводным камням — **отладка корутин**: почему стек-трейсы выглядят непривычно (те самые `.actor`-фреймы, что мелькали в выводе ASan — это трансформированное компилятором тело корутины, разбитое на части), как отладчик показывает приостановленную корутину, где искать локальные переменные корутины в памяти, и почему точки останова ведут себя не так, как в обычных функциях. Разберём?


---


## Этап 7. Подводные камни

- Хочешь, разберём следующий подводный камень — `co_await` временных объектов и время жизни внутри awaiter (тонкая разновидность той же проблемы), или аллокации фрейма и попытки их устранить (HALO, кастомный `operator new` для промиса)?
- 
- Время жизни объектов и аргументов относительно coroutine frame
- Аллокации и попытки их избежать (Heap Allocation Elision Optimization, HALO)
- Отладка: почему стек-трейсы корутин «непривычны»

---
