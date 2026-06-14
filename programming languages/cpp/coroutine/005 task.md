---
tags:
  - programming-language
  - cpp
  - coroutine
---
[[programming languages/cpp/coroutine/_|<=]]

## Task — это тип возврата корутины

Корутина должна возвращать тип, у которого есть `promise_type`. `Task` — это типичное имя для такой обёртки, представляющей **асинхронную задачу, которая когда-нибудь вернёт результат типа `T`**.

```cpp
Task<int> compute() {
    co_return 42;
}
```

Здесь `Task<int>` — это объект, который получает вызывающий. Сама корутина при этом может ещё не выполниться (ленивая модель) — `Task` лишь даёт ручку для управления ею и получения результата через `co_await`.

Имя `Task` — это конвенция (пришедшая из C#, где есть `Task<T>`), а не требование языка. Можно было назвать его `Async`, `Future`, `Lazy` — суть та же.

## Почему его нет в стандарте

Стандарт C++20 дал только низкоуровневую инфраструктуру (`promise_type`, `coroutine_handle`, awaiter'ы), но **готовых типов вроде `Task` не предоставил**. Предполагалось, что их дадут библиотеки. На практике используют:

- **cppcoro** — `cppcoro::task<T>`
- **Boost.Asio** — `boost::asio::awaitable<T>`
- **libunifex / std::execution** — sender'ы
- собственные реализации

`std::generator` (для `co_yield`) появился только в C++23, а стандартного `std::task` нет до сих пор — он обсуждается для будущих стандартов.

## Минимальная реализация Task

Чтобы `Task` перестал быть «магией», вот упрощённая рабочая версия для `co_return` значения:

```cpp
#include <coroutine>
#include <optional>
#include <utility>

template <typename T>
struct Task {
    struct promise_type {
        std::optional<T> result;
        std::coroutine_handle<> continuation;   // кто нас ждёт (для co_await)

        Task get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        // ленивая модель: не начинаем выполнение до первого resume
        std::suspend_always initial_suspend() noexcept { return {}; }

        // при завершении — возобновляем того, кто нас co_await-ил
        auto final_suspend() noexcept {
            struct FinalAwaiter {
                bool await_ready() noexcept { return false; }
                std::coroutine_handle<> await_suspend(
                    std::coroutine_handle<promise_type> h) noexcept {
                    auto cont = h.promise().continuation;
                    return cont ? cont : std::noop_coroutine();
                }
                void await_resume() noexcept {}
            };
            return FinalAwaiter{};
        }

        void return_value(T v) { result = std::move(v); }
        void unhandled_exception() { std::terminate(); }
    };

    std::coroutine_handle<promise_type> h;

    explicit Task(std::coroutine_handle<promise_type> handle) : h(handle) {}
    Task(Task&& other) noexcept : h(std::exchange(other.h, {})) {}
    ~Task() { if (h) h.destroy(); }

    // делает Task awaitable — чтобы можно было co_await task
    bool await_ready() noexcept { return false; }
    std::coroutine_handle<> await_suspend(std::coroutine_handle<> caller) noexcept {
        h.promise().continuation = caller;   // запомнить, кто нас ждёт
        return h;                            // symmetric transfer: передать управление нам
    }
    T await_resume() { return std::move(*h.promise().result); }
};
```

## Разбор ключевых деталей

**`get_return_object`** — создаёт сам объект `Task`, который вернётся вызывающему в момент создания корутины.

**`initial_suspend` → `suspend_always`** — делает `Task` _ленивым_: корутина не начинает выполняться сразу при вызове, а ждёт первого `co_await`/`resume`. (Если бы стоял `suspend_never`, выполнение началось бы немедленно — это «eager» модель.)

**`final_suspend` + continuation** — это сердце композиции задач. Когда `inner` завершается, она через symmetric transfer возобновляет `outer`, который её ждал (об этом был пункт про symmetric transfer). `std::noop_coroutine()` — заглушка на случай, если никто не ждёт.

**Методы `await_ready/suspend/resume` на самом `Task`** — делают `Task` _awaitable_, то есть позволяют писать `co_await someTask`. Возврат handle из `await_suspend` — это symmetric transfer: управление напрямую передаётся вложенной корутине без роста стека.

## Чем Task отличается от Generator

| |`Task<T>`|`Generator<T>`|
|---|---|---|
|Ключевое слово|`co_return` (и `co_await` внутри)|`co_yield`|
|Возвращает|одно значение (однократно)|поток значений (многократно)|
|Назначение|асинхронная задача|ленивая последовательность|
|Завершается после выдачи|да|нет, продолжается|

Оба — пользовательские обёртки над одной и той же машинерией, отличаются содержимым `promise_type` (`return_value` vs `yield_value`) и тем, как с ними работают снаружи.

## Итог

`Task` — это **условная обёртка-тип возврата** для асинхронной корутины, не часть стандарта. Подразумевая «некий тип с правильным `promise_type`». Сама по себе корутина — это протокол; `Task` — конкретная реализация одного из вариантов этого протокола.
