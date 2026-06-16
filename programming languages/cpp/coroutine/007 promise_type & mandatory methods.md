---
tags:
  - programming-language
  - cpp
  - coroutine
---
[[programming languages/cpp/coroutine/_|<=]]

`promise_type` — это «пульт управления» корутиной: компилятор обращается к нему в каждой ключевой точке жизненного цикла. 

## get_return_object

**Когда вызывается:** один раз, в самом начале — сразу после создания coroutine frame, ещё до выполнения тела корутины.

**Что делает:** создаёт объект, который будет **возвращён вызывающему**. То есть результат `get_return_object()` — это то, что получит код, написавший `auto x = myCoroutine();`.

```cpp
MyTask get_return_object() {
    return MyTask{std::coroutine_handle<promise_type>::from_promise(*this)};
}
```

Ключевая деталь — `from_promise(*this)`. Промис живёт _внутри_ coroutine frame, и по ссылке на промис можно восстановить handle всей корутины. Это мост: промис «знает себя», создаёт handle и упаковывает его в возвращаемый объект, чтобы вызывающий мог управлять корутиной (возобновлять, проверять `done`, уничтожать).

Тонкость порядка: `get_return_object()` вызывается **до** `initial_suspend()`. Поэтому возвращаемый объект формируется раньше, чем тело корутины получит шанс выполниться.

## initial_suspend

**Когда вызывается:** сразу после `get_return_object()`, перед первой строкой тела корутины. Результат — awaitable, который компилятор `co_await`-ит.

**Что определяет:** приостановиться ли корутине **перед началом** выполнения тела. Это развилка между двумя моделями:

```cpp
// ЛЕНИВАЯ (lazy): корутина застывает перед телом,
// выполнится только при первом resume/co_await
std::suspend_always initial_suspend() noexcept { return {}; }

// НЕМЕДЛЕННАЯ (eager): тело начинает выполняться сразу при вызове корутины
std::suspend_never initial_suspend() noexcept { return {}; }
```

Этот выбор — фундаментальный для поведения типа:

| |`suspend_always` (lazy)|`suspend_never` (eager)|
|---|---|---|
|Тело стартует|при первом `resume`/`co_await`|сразу при вызове|
|Кто подходит|`Task` (композиция), `Generator`|«выстрелил и забыл», простые случаи|
|Контроль|вызывающий решает, когда запускать|начинается немедленно|

В наших примерах: `Generator` и ленивый `Task` использовали `suspend_always` (нужно дождаться запроса), а eager-`Task` для `main` — `suspend_never` (чтобы результат был готов сразу).

`noexcept` важен: если `initial_suspend` бросит, обработка крайне затруднена, поэтому его всегда делают `noexcept`.

## Полный список обязательных методов

`promise_type` не скомпилируется без всех этих членов:

|Метод|Назначение|Когда вызывается|
|---|---|---|
|`get_return_object()`|создать объект для вызывающего|в начале, до тела|
|`initial_suspend()`|приостановиться ли перед телом|после `get_return_object`|
|`final_suspend() noexcept`|приостановиться ли после тела|после завершения тела|
|`unhandled_exception()`|обработать вылетевшее исключение|при исключении в теле|
|`return_value(v)` **или** `return_void()`|обработать `co_return`|при `co_return`|

Плюс — в зависимости от того, что использует тело: `yield_value(v)` для `co_yield` (был в `Generator`). Для чисто `co_await`-корутины он не нужен.

Замечу про `final_suspend() noexcept` — он обязан быть `noexcept`, иначе программа ill-formed. Выбор `suspend_always` здесь оставляет frame живым после завершения (чтобы успеть прочитать результат — кто-то должен потом вызвать `destroy()`); `suspend_never` уничтожает frame автоматически.

## Минимальный рабочий пример

Самый компактный честный промис — на корутине, которая ничего не возвращает и просто выполняет тело.

```cpp
#include <iostream>
#include <coroutine>

struct Coro {
    struct promise_type {
        Coro get_return_object() { return {}; }
        std::suspend_never initial_suspend() noexcept { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
};

Coro hello() {
    std::cout << "before\n";
    co_return;
}

int main() {
    hello();
    std::cout << "after\n";

    return 0;
}
```

```
before
after
```

## Почему именно так и что произойдёт при изменениях

**`get_return_object()` возвращает `{}`** — здесь `Coro` пустой (ничем не управляет, handle не хранит), поэтому достаточно пустого объекта. Как только понадобится управлять корутиной снаружи (как в `Task`/`Generator`), сюда вернётся объект с handle через `from_promise(*this)`.

**`initial_suspend → suspend_never` (eager)** — тело `hello()` выполняется прямо в точке вызова. Вывод `before` появляется _до_ `after`. Если заменить на `suspend_always`, то `hello()` создаст корутину, но тело не выполнится (никто не вызывает `resume`), и `before` вообще не напечатается — выведется только `after`.

**`final_suspend → suspend_never`** — после `co_return` frame уничтожается автоматически. Это безопасно здесь, потому что `Coro` не хранит handle и не пытается его потом использовать. Если бы мы (как в `Task`) читали результат после завершения, понадобился бы `suspend_always` + ручной `destroy()` — иначе чтение из уничтоженного frame даст UB.

**`unhandled_exception()` пустой** — если бы тело бросило исключение, оно было бы проглочено. В реальном коде сюда кладут `std::terminate()` или сохранение исключения через `std::current_exception()` для последующего проброса в `await_resume`/`get`.

Можно проверить связку наглядно: меняешь `initial_suspend` на `suspend_always` и убеждаешься, что `before` исчезает из вывода — это лучший способ прочувствовать, что делает каждый метод.
