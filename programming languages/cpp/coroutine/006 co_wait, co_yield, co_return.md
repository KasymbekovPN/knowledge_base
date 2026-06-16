---
tags:
  - programming-language
  - cpp
  - coroutine
---
[[programming languages/cpp/coroutine/_|<=]]

**Главный принцип:** функция становится корутиной автоматически, если в её **теле** встречается хотя бы одно из `co_await`, `co_yield`, `co_return`. Не нужно никакого специального атрибута или ключевого слова в сигнатуре — наличие любого из трёх в теле превращает обычную функцию в корутину. Компилятор сам это распознаёт и генерирует всю машинерию (фрейм, `promise_type`, точки приостановки).

## co_return — завершение

Завершает корутину, опционально возвращая значение. Аналог обычного `return`, но для корутины.

Что происходит под капотом: компилятор превращает `co_return expr;` в вызов `promise.return_value(expr)`, а голый `co_return;` — в `promise.return_void()`. В `promise_type` должен быть ровно один из этих методов (наличие неподходящего — ошибка компиляции). После этого выполняется `final_suspend()`.

В отличие от обычной функции, в корутине **нельзя использовать обычный `return`** — только `co_return`.

```cpp
Task<int> compute() {
    co_return 42;     // вызовет promise.return_value(42)
}

Task<void> doStuff() {
    co_return;        // без значения → promise.return_void()
}
```

```cpp
#include <coroutine>
#include <iostream>
#include <format>

template <typename T>
struct Task {
    struct promise_type {
        T value;

        Task get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_never  initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend()  noexcept { return {}; }
        void return_value(T v) { value = v; }
        void unhandled_exception() { std::terminate(); }
    };

    std::coroutine_handle<promise_type> h;

    explicit Task(std::coroutine_handle<promise_type> handle) : h(handle) {}
    ~Task() { if (h) h.destroy(); }

    T get() { return h.promise().value; }
};

Task<int> compute() {
    co_return 42;
}

int main() {
    Task<int> task = compute();
    std::cout << std::format("Result: {}\n", task.get());

    return 0;
}
```

```
Result: 42
```
## co_await — приостановка до готовности значения

Приостанавливает корутину, пока ожидаемое (awaitable) не будет готово, и возвращает результат. Это основа асинхронности.

```cpp
Task handle(Connection c) {
    auto data = co_await c.async_read();    // приостановка до прихода данных
    process(data);
    co_await c.async_write(response);       // приостановка до завершения записи
}
```

Что происходит под капотом — `co_await expr` разворачивается в работу с awaiter'ом через три его метода (это подробно в следующем пункте плана):

- `await_ready()` — готово ли уже? Если да — не приостанавливаемся.
- `await_suspend()` — что сделать при приостановке (например, запланировать возобновление по завершении I/O).
- `await_resume()` — что вернуть как результат выражения `co_await`.

`co_await` — это оператор, и его можно перегружать, но об этом ниже.

```cpp
#include <iostream>
#include <format>
#include <coroutine>
#include <optional>
#include <utility>
#include <thread>
#include <future>

struct DetachedTask {
    struct promise_type {
        DetachedTask get_return_object() { return {}; }
        std::suspend_never initial_suspend() noexcept { return {}; } // eager
        std::suspend_never final_suspend() noexcept { return {}; } // selfdestroy
        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };
};

template<typename T>
struct Task {
    struct promise_type {
        std::optional<T> result;
        std::coroutine_handle<> continuation;

        Task get_return_object() {
            return Task{
                std::coroutine_handle<promise_type>::from_promise(*this)
            };
        }

        // lazy model
        std::suspend_always initial_suspend() noexcept { return {}; }

        // when finished, we resume the one who co_awaited us
        auto final_suspend() noexcept {
            struct FinalAnswer {
                bool await_ready() noexcept { return false; }
                std::coroutine_handle<> await_suspend(
                    std::coroutine_handle<promise_type> h
                ) noexcept {
                    auto cont = h.promise().continuation;
                    return cont ? cont : std::noop_coroutine();
                }
                void await_resume() noexcept {};
            };
            return FinalAnswer{};
        }

        void return_value(T v)  { result = std::move(v); }
        void unhandled_exception() { std::terminate(); }
    };

    std::coroutine_handle<promise_type> h;

    explicit Task(std::coroutine_handle<promise_type> _h): h{_h} {}
    Task(Task&& _other) noexcept: h{std::exchange(_other.h, {})} {}
    ~Task() { if(h) h.destroy(); }

    // making task awaitable - may use co_await
    bool await_ready() noexcept { return false; }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<> caller) noexcept {
        h.promise().continuation = caller;
        return h;
    }

    T await_resume() { return std::move(*h.promise().result); }
};

template<typename T>
T sync_wait(Task<T> _task) {
    std::promise<T> p;
    std::future<T> f = p.get_future();

    auto&& runner = [&]() -> DetachedTask {
        p.set_value(co_await _task);
    }();

    f.wait();
    return f.get();
}

Task<int> compute() {
    co_return 42;
}

int main() {
    int result{};
    {
        [&]() {
            return std::jthread{[task = compute(), &result]() mutable {
                result = sync_wait(std::move(task));
            }};
        }();
    }
    std::cout << std::format("result: {}\n", result);

    return 0;
}
```

```
Result: 42
```

## co_yield — выдать значение с приостановкой

Возвращает промежуточное значение и приостанавливается, сохраняя возможность продолжить. Основа генераторов (ленивых последовательностей).

```cpp
#include <iostream>
#include <format>
#include <coroutine>
#include <cstdint>
#include <utility>

template <typename T>
struct Generator {
    struct promise_type {
        T current;

        Generator get_return_object() {
            return Generator{
                std::coroutine_handle<promise_type>::from_promise(*this)
            };
        }

        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {};}
        std::suspend_always yield_value(T v) noexcept {
            current = v;
            return {};
        }
        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };

    std::coroutine_handle<promise_type> h;

    explicit Generator(std::coroutine_handle<promise_type> _h): h{_h} {}
    Generator(Generator&& _other) noexcept: h{std::exchange(_other.h, {})} {}
    ~Generator() { if (h) h.destroy(); }

    bool next() {
        h.resume();
        return !h.done();
    }
    T value() const { return h.promise().current; }
};

Generator<uint64_t> fibonacci() {
    uint32_t a = 0, b = 1;
    while (true) {
        co_yield a;
        uint64_t next = a + b;
        a = b;
        b = next;
    }
}

int main() {
    auto&& gen = fibonacci();
    for (size_t i{}; i < 10; i++) {
        gen.next();
        std::cout << std::format("{} ", gen.value());
    }
    std::cout << "\n";

    return 0;
}
```

```
0 1 1 2 3 5 8 13 21 34
```

Что происходит под капотом: `co_yield expr` — это синтаксический сахар, эквивалентный `co_await promise.yield_value(expr)`. То есть `co_yield` определяется через `co_await`: значение передаётся в `yield_value`, который обычно сохраняет его в промисе и возвращает awaiter, приостанавливающий корутину.

Ключевое отличие от `co_return`: после `co_yield` корутина **не завершена** — её можно возобновить, и она продолжит с места приостановки. После `co_return` корутина завершается.

## Сводная таблица

|Ключевое слово|Что делает|Завершает корутину?|Разворачивается в|
|---|---|---|---|
|`co_await e`|Приостановка до готовности `e`, вернуть результат|Нет|работу с awaiter (`await_ready/suspend/resume`)|
|`co_yield e`|Выдать `e` и приостановиться|Нет|`co_await promise.yield_value(e)`|
|`co_return e`|Завершить, вернув `e`|Да|`promise.return_value(e)` / `return_void()`|

Обратите внимание: `co_yield` — это надстройка над `co_await`, а оба они и `co_return` опираются на методы `promise_type`. Корутины в C++ — это, по сути, протокол: ключевые слова превращаются в вызовы методов промиса и awaiter'ов.

## Важные ограничения — что НЕ может быть корутиной

Не всякую функцию можно сделать корутиной, даже добавив `co_*`:

- **`constexpr`-функции** — нельзя.
- **Конструкторы и деструкторы** — нельзя.
- **`main`** — нельзя.
- **Функции с переменным числом аргументов** (C-style `...`) — нельзя.
- **Функции, возвращающие `auto` без явного типа-обёртки**, в общем случае проблематичны — тип возврата корутины должен быть конкретным типом, для которого определён `promise_type` (через `std::coroutine_traits`). `auto` с выводом из `co_return` не работает так, как в обычных функциях.

Зато корутиной **может** быть: обычная функция, функция-член класса, лямбда (с осторожностью к времени жизни захватов — это в пункте про подводные камни).

## Что определяет тип возврата

Тип возврата корутины — не произвольный. Он должен быть типом, у которого компилятор найдёт `promise_type` (напрямую как вложенный тип или через специализацию `std::coroutine_traits`). Именно `promise_type` задаёт всё поведение: что вернуть вызывающему, приостанавливаться ли в начале и в конце, как обработать `co_return`/`co_yield`/исключения.

```cpp
Task<int> f() {       // Task<int> обязан иметь Task<int>::promise_type
    co_return 1;
}
```

Поэтому «просто написать `co_return`» недостаточно для рабочей программы — нужен корректный тип-обёртка с промисом. Голые ключевые слова делают функцию корутиной _синтаксически_, но скомпилируется она только если тип возврата предоставляет требуемый протокол.
