---
tags:
  - programming-language
  - cpp
  - coroutine
---
[[programming languages/cpp/coroutine/_|<=]]

## awaiter хранит ссылку на временный объект

### Когда временный в co_await живёт, а когда нет

Ключевое правило: **временные объекты, созданные в полном выражении `co_await`, живут до конца этого полного выражения — включая приостановку и возобновление.** То есть временный объект-аргумент в `co_await waitOn(std::string("..."))` доживает до того момента, как `co_await` целиком завершится (после `await_resume`). Поэтому пример выше формально **корректен** — стандарт гарантирует жизнь временного до конца `co_await`.

Вот почему ASan молчит: продление жизни временных в `co_await` работает как обычное правило C++ для полных выражений. Это была моя проверка на честность — не всякий `co_await` временного ломается.

**Проблема возникает, когда ссылка переживает полное выражение** — например, awaiter сохраняет ссылку, а корутина приостанавливается _между_ созданием awaiter и его использованием так, что временный успевает умереть. Покажу настоящую поломку: awaiter, который через `await_transform` или промежуточное хранение держит ссылку на временный объект _дольше_, чем длится `co_await`.

Самый чистый способ воспроизвести реальную проблему — это когда awaitable связан со ссылкой, а возобновление откладывается на **другую** точку (не внутри того же полного выражения):Вот теперь настоящая поломка — `heap-use-after-free`. Это и есть реальная, частая ловушка. Разберу, чем она отличается от первого «безопасного» случая.

## Что произошло

```cpp
Task processor(std::string_view sv) {        // sv копируется во frame...
    std::cout << "[before suspend] sv = " << sv << '\n';   // живо
    co_await std::suspend_always{};           // ПРИОСТАНОВКА
    std::cout << "[after suspend]  sv = " << sv << '\n';   // sv висит!
}

Task t = processor(std::string("temporary..."));
// ^ временная std::string умирает здесь, в конце выражения
t.resume();   // sv внутри frame смотрит на освобождённую память -> UB
```

Тонкость в том, что `sv` (`string_view`) **скопирован во frame по значению** — сам объект `string_view` живёт столько же, сколько корутина. Но `string_view` — это лишь указатель + длина; он **не владеет** строкой. Скопировался указатель, а данные (временная `std::string`) умерли в конце выражения создания корутины. Корутина приостановилась и пережила их — указатель повис.

## Чем это отличается от первого примера

| |Первый пример (`waitOn`)|Второй пример (`processor`)|
|---|---|---|
|Что с временным|живёт до конца `co_await`|умирает в конце выражения создания корутины|
|Приостановка|внутри того же полного выражения|корутина пережила выражение|
|Итог|корректно (стандарт продлевает)|UB (временного уже нет)|

Правило, которое всё объясняет: **временные продлеваются только до конца своего полного выражения.** В первом случае `co_await` целиком был этим выражением — временный дожил. Во втором временный жил лишь до `;` в `processor(std::string(...))`, а корутина приостановилась и пережила эту точку — ссылка повисла.

## Это та же болезнь, что висячие захваты

Заметь параллель с прошлым разбором: и там, и тут проблема одна — **корутина (frame) переживает объект, на который ссылается извне**. Разница лишь в том, _что именно_ ссылается:

- лямбда: захват по ссылке/значению в объекте-замыкании;
- здесь: ссылочный тип (`string_view`, `span`, `T&`), скопированный во frame, но смотрящий на чужие умершие данные.

`string_view` и `std::span` особенно опасны как параметры корутин именно потому, что выглядят как «значение» (копируются во frame), а ведут себя как ссылка (не продлевают данные).

## Правила

**1. Не передавай ссылочно-подобные типы в корутины, которые приостанавливаются.** `std::string_view`, `std::span`, `const T&`, указатели — всё, что не владеет данными, рискованно как параметр корутины, переживающей вызывающего.

**2. Передавай владеющие типы по значению.** `std::string` вместо `std::string_view`, `std::vector<T>` вместо `std::span<T>`. Тогда данные копируются во frame и живут с корутиной:

```cpp
Task processor(std::string sv);        // OK: владеет, копия во frame
Task processor(std::string_view sv);   // ОПАСНО: смотрит на чужое
```

**3. Для awaiter — не храни ссылки, которые могут пережить co_await.** Если awaiter сохраняет `const T&`, убедись, что объект жив всё время от `await_suspend` до `await_resume`. При сомнении — храни по значению.

**4. Помни про границу полного выражения.** Временный в `co_await expr` жив до конца `co_await`, но не дольше. Если awaitable «утекает» из выражения (сохраняется, передаётся), его ссылки на временные повиснут.

## Итог по этому подводному камню

`co_await` временного объекта сам по себе безопасен — стандарт держит временный живым до конца полного выражения `co_await`. Опасность не в самом `co_await`, а в **ссылочно-подобных типах, которые переживают своё полное выражение**: `string_view`/`span`/`T&`, скопированные во frame, продолжают смотреть на данные, которые уже умерли. Лечение то же, что и везде в корутинах: владеть данными во frame (передавать владеющие типы по значению), а не ссылаться на чужое время жизни.

```cpp
// clang++.exe -std=c++26 -O0 -fsanitize=address .\demo_str_ref.cpp -o .\demo_str_ref.exe
// компилируя с -fsanitize=address, нужно либо копировать clang_rt.asan_dynamic-x86_64.dll рядом с .exe,
// либо один раз добавить путь к нему в PATH: $env:PATH += ";C:\Program Files\LLVM\lib\clang\22\lib\windows"

#include <iostream>
#include <format>
#include <string>
#include <coroutine>
#include <utility>

struct Task {
    struct promise_type {
        Task get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_never initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };

    std::coroutine_handle<promise_type> h;

    explicit Task(std::coroutine_handle<promise_type> _h): h{_h} {}
    ~Task() { if (h) h.destroy(); }
};

struct BadAwaiter {
    // ССЫЛКА — опасно, если объект временный
    const std::string& ref;
    bool await_ready() const noexcept { return false; }
    void await_suspend(std::coroutine_handle<> _h)  const noexcept {
        _h.resume(); // имитируем приостановку+возобновление
    }
    // к моменту resume временный объект, на который смотрит ref, может быть мёртв
    std::string await_resume() const { return ref; } // читаем висячую ссылку
};

// helper, возвращающий awaiter по ссылке на ВРЕМЕННЫЙ аргумент
BadAwaiter waitOn(const std::string& s) {
    return BadAwaiter{s};
}

Task run() {
    // co_await с ВРЕМЕННЫМ объектом: std::string{"hello"} — prvalue
    std::string result = co_await waitOn(std::string("temporary"));
    std::cout << std::format("got: {}\n", result);

    co_return;
}

int main() {
    run();

    return 0;
}
```

```cpp
// clang++.exe -std=c++26 -O0 -fsanitize=address .\demo_string_view.cpp -o .\demo_string_view.exe

// компилируя с -fsanitize=address, нужно либо копировать clang_rt.asan_dynamic-x86_64.dll рядом с .exe,

// либо один раз добавить путь к нему в PATH: $env:PATH += ";C:\Program Files\LLVM\lib\clang\22\lib\windows"

#include <iostream>
#include <format>
#include <string>
#include <string_view>
#include <coroutine>
#include <utility>

// Task, чей promise ХРАНИТ string_view (ссылку на чужие данные)
struct Task {
    struct promise_type {
        std::string_view stored;
        Task get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };

    std::coroutine_handle<promise_type> h;

    explicit Task(std::coroutine_handle<promise_type> _h): h{_h} {}
    Task(Task&& _other) noexcept: h{std::exchange(_other.h, {})} {}
    ~Task() { if(h) h.destroy(); }
    void resume() { h.resume(); }
    std::string_view stored() const { return h.promise().stored; }
};

// Корутина принимает string_view ПО ЗНАЧЕНИЮ, но string_view сам по себе
// это лишь указатель+длина: он НЕ продлевает жизнь строки, на которую смотрит.
Task processor(std::string_view _sv) {
    std::cout << std::format("[before suspend] sv: {}\n", _sv); // тут ещё живо
    co_await std::suspend_always();
    // ... к моменту возобновления временная строка может быть мертва ...
    std::cout << std::format("[after suspend] sv: {}\n", _sv);// читаем висячий sv
    co_return;
}

int main() {
    // Передаём string_view на ВРЕМЕННУЮ строку.
    // Временная std::string живёт только до конца ЭТОГО выражения (точка с запятой),
    // а корутина приостановилась внутри и переживёт её.

    std::string_view s0{"temporary 0"};
    Task t0 = processor(s0);
    Task t1 = processor(std::string{"temporary 1"}); // здесь std::string - временный объект

    std::cout << "resuming...\n";
    t0.resume();
    t1.resume();

    // <-- временная std::string УЖЕ уничтожена здесь, но корутина приостановлена
    std::cout << "resuming...\n";
    t0.resume();
    t1.resume(); // возобновляем -> sv смотрит на освобождённую память -> UB

    return 0;
}
```
