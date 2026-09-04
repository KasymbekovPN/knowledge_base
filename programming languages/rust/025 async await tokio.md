---
tags:
  - programming-language
  - rust
---
[[programming languages/rust/_|<=]]

## Общий фундамент: оба — stackless coroutines

И Rust `async`/`await`, и C++20 корутины — **stackless** (в отличие от stackful, как горутины Go или `boost::context`/fibers). Это значит: приостанавливается только локальный фрейм текущей функции, не весь call stack. Если `async fn b()` вызывает `async fn a()` через `.await`, то `b` тоже обязана быть `async` — точно как в C++20, где вызов корутины из обычной функции без `co_await` просто не приостановит вызывающую сторону.

## `Future` — трейт вместо кастомизации через `promise_type`

Вот где начинается принципиальная разница философии. В C++20 ты сам пишешь `promise_type` с набором customization points (`get_return_object`, `initial_suspend`, `final_suspend`, `return_value`/`return_void`, `unhandled_exception`, опционально `await_transform`) — компилятор использует твою реализацию, чтобы понять, как вести себя. В Rust этого нет вообще: `Future` — обычный трейт с **одним** методом:

```rust
trait Future {
    type Output;
    fn poll(self: Pin<&mut Self>, cx: &mut Context<'_>) -> Poll<Self::Output>;
}

enum Poll<T> {
    Ready(T),
    Pending,
}
```

Компилятор сам генерирует `Future`-реализацию (скрытую struct-state-machine) из тела `async fn` — тебе никогда не нужно писать свой `promise_type`-аналог для обычного использования. Кастомизация нужна только если пишешь **свой** Future вручную (что мы сейчас и сделали) — и там кастомизировать нечего, кроме самого `poll`.

## `poll()` (pull) vs `resume()` (push) — вот ключевая архитектурная разница

В C++20 `coroutine_handle::resume()` просто **продолжает выполнение** до следующей точки приостановки — никакого "статуса готовности" метод не возвращает, вся информация о готовности идёт через протокол `await_ready()`/`await_suspend()`/`await_resume()` awaiter'а, с которым имеет дело именно вызывающая точка `co_await`, а не сам механизм резюмирования.

В Rust `poll()` **явно** возвращает `Poll::Ready(T)` или `Poll::Pending` — это pull-модель: исполнитель **спрашивает** Future "ты готов?", вместо того чтобы просто продолжать исполнение вслепую. Я написал и прогнал свой Future вручную, чтобы показать механику голыми руками (аналог твоего написания собственного awaiter'а):

```rust
use std::future::Future;
use std::pin::Pin;
use std::task::{Context, Poll, Waker};
use std::sync::{Arc, Mutex};
use std::thread;
use std::time::Duration;

struct Delay {
    shared: Arc<Mutex<SharedState>>,
}

struct SharedState {
    completed: bool,
    waker: Option<Waker>,
}

impl Future for Delay {
    type Output = String;

    fn poll(self: Pin<&mut Self>, cx: &mut Context<'_>) -> Poll<Self::Output> {
        let mut shared = self.shared.lock().unwrap();
        if shared.completed {
            Poll::Ready("готово!".to_string())
        } else {
            shared.waker = Some(cx.waker().clone()); // сохраняем continuation
            Poll::Pending
        }
    }
}

impl Delay {
    fn new(duration: Duration) -> Self {
        let shared = Arc::new(Mutex::new(SharedState { completed: false, waker: None }));
        let thread_shared = Arc::clone(&shared);
        thread::spawn(move || {
            thread::sleep(duration);
            let mut s = thread_shared.lock().unwrap();
            s.completed = true;
            if let Some(waker) = s.waker.take() {
                waker.wake(); // "разбуди меня" -- сигнал исполнителю перепроверить
            }
        });
        Delay { shared }
    }
}

#[tokio::main]
async fn main() {
    println!("до await");
    let result = Delay::new(Duration::from_millis(200)).await;
    println!("после await: {result}");
}
```

Вывод: `до await` → (пауза 200мс) → `после await: готово!`.

## `Waker` — прямой аналог твоего continuation/symmetric transfer, но развязанный

`cx.waker()` — это то, что в Boost.Asio выполняет completion handler, вызывающий `coroutine_handle::resume()` (или отдающий следующий handle через symmetric transfer, чтобы избежать роста стека при цепочках). Механически цель та же: "скажи исполнителю, когда продолжать" — но в Rust это **явный, передаваемый объект** (`Waker`), который любой Future может клонировать, сохранить, передать в другой поток, вызвать `.wake()` откуда угодно. В Boost.Asio эта роль обычно спрятана внутри `io_context` и completion handler'ов, не выведена как отдельная сущность первого класса, с которой можно работать напрямую в пользовательском коде так же свободно.

## `.await` — то же самое разворачивание в state machine, что `co_await`

```rust
async fn task(name: &str, ms: u64) {
    println!("{name}: старт");
    sleep(Duration::from_millis(ms)).await;
    println!("{name}: финиш");
}
```

Каждая `.await`-точка — это ровно один переход состояния в компилятором сгенерированном `enum`, буквально то же самое, что происходит с coroutine frame в C++20 на каждом `co_await`. Я прогнал три конкурентные задачи с разным временем сна:

```rust
let h1 = tokio::spawn(task("A", 300));
let h2 = tokio::spawn(task("B", 100));
let h3 = tokio::spawn(task("C", 200));
let _ = tokio::join!(h1, h2, h3);
```

Вывод:

```
A: старт
B: старт
C: старт
B: финиш
C: финиш
A: финиш
```

Все три стартуют немедленно (конкурентно), финишируют в порядке готовности своих таймеров — ровно то поведение, которого ты ожидаешь от кооперативного планировщика, с которым уже работал в контексте Boost.Asio/cppcoro.

## Аллокация: главное практическое отличие от C++20

Вот здесь разница действительно существенная, а не косметическая:

```rust
async fn step_a() -> i32 { 1 }
async fn step_b() -> i32 {
    let x = step_a().await;
    x + step_a().await
}

fn main() {
    let fut = step_b(); // Future ещё не запущен -- ленивый
    println!("size_of Future для step_b() = {} байт", size_of_val(&fut));
}
```

Вывод: **8 байт**. Никакой кучи не тронуто — `step_b`'s Future — это просто enum-state-machine на стеке, композиция `step_a().await` внутри неё **не требует аллокации** вообще.

В C++20 coroutine frame **почти всегда** аллоцируется в куче (спецификация формально позволяет компилятору elidировать аллокацию через HALO — Heap Allocation eLision Optimization — если он статически докажет, что время жизни фрейма не выходит за пределы вызывающей функции, но это **оптимизация**, не гарантия — во многих реальных случаях, особенно через границы TU или с виртуальными вызовами, HALO не срабатывает, и ты платишь за `operator new` на каждый вызов корутины). В Rust это **гарантия языка**, а не оптимизация, на которую можно надеяться: `async fn`-Future не требует аллокации для собственной композиции, пока ты не решишь **сам** стереть тип (`Box<dyn Future>`) или передать в `tokio::spawn` (который требует `'static`, и рантайм может — в зависимости от реализации — заассайнить задачу в куче для хранения в очереди исполнителя, но сама механика await-цепочки внутри задачи остаётся аллокационно-бесплатной).

## `Pin` — то же самое решение проблемы self-referential структур, что и стабильный адрес coroutine frame в C++

Вот прямая техническая параллель, которую ты, скорее всего, оценишь: причина, по которой C++20 coroutine frame почти всегда в куче — фрейм может содержать указатели/ссылки на собственные локальные переменные (типичный case: `auto x = foo(); auto& y = x; co_await something(y);`), и адрес фрейма должен оставаться стабильным между вызовами `resume()`, иначе внутренние ссылки станут dangling.

Rust решает **ту же самую** проблему иначе — не гарантией "фрейм в куче с фиксированным адресом", а через тип `Pin<P>`, который **статически запрещает** перемещение значения после первого `poll()`:

```rust
fn poll(self: Pin<&mut Self>, cx: &mut Context<'_>) -> Poll<Self::Output>
```

`Pin<&mut Self>` — это гарантия компилятора "этот `Self` больше никогда не переместится в памяти, пока жив" — именно поэтому Rust-Future **может** безопасно жить на стеке (не обязан аллоцироваться), даже если он self-referential внутри: гарантия стабильности адреса дана через типовую систему, а не через физическое размещение в куче. Это прямой ответ на ту же архитектурную проблему, которую в C++ решили мандатом на heap allocation (с опциональным elision) — просто более гибкое решение: стабильность адреса доказывается статически, а не обеспечивается физически фиксированным местом хранения.

## Исполнитель (executor): в std его нет вообще — как `io_context`, только явно опциональный

`async`/`await`-синтаксис и трейт `Future` — часть `core`/`std`, но **самого исполнителя в std нет**. Ты обязан принести свой рантайм — `tokio`, `async-std`, `smol` — ровно так же, как `boost::asio::io_context` не часть стандарта C++, а библиотечное решение поверх языковой фичи корутин. `#[tokio::main]` (уже видел этот атрибутный процедурный макрос в теме про макросы) — это то, что превращает `async fn main()` в обычный `fn main()`, который создаёт `tokio`-рантайм и запускает на нём Future. Кстати, раз ты упоминал P2300/std::execution (sender-receiver, C++26) в своих прошлых заметках — это фактически попытка C++ сделать то же самое, что Rust сделал ещё в 2019: явно вынести исполнение в переиспользуемую, стандартизируемую абстракцию, а не зашивать его в конкретную библиотеку типа Boost.Asio.

## `tokio::spawn` — требует `'static + Send`, `thread::scope`-аналога нет

```rust
let h1 = tokio::spawn(good_task(1, Arc::clone(&data)));
```

Как и `thread::spawn` (прошлая тема), `tokio::spawn` требует `'static` — задача может пережить функцию, из которой запущена. Но требует ещё и `Send` — задача может быть переброшена планировщиком между рабочими потоками исполнителя между `.await`-точками (work-stealing, как в tokio multi-thread рантайме). Это прямо связано со следующим примером.

## Классическая ловушка: `std::sync::Mutex` через точку `.await`

Вот где Rust-компилятор ловит целый класс багов, которые в C++/Boost.Asio легко просмотреть:

```rust
async fn bad_task(id: u32, data: Arc<std::sync::Mutex<i32>>) {
    let mut guard = data.lock().unwrap(); // держим std-мьютекс...
    sleep(Duration::from_millis(100)).await; // ...и ЗАСЫПАЕМ, держа его
    *guard += 1;
}
```

Компилятор **отказался это компилировать**:

```
error: future cannot be sent between threads safely
has type `std::sync::MutexGuard<'_, i32>` which is not `Send`
await occurs here, with `mut guard` maybe used later
required by a bound in `tokio::spawn`
```

Механика: `guard` "жив" через точку `.await`, значит он становится частью сгенерированной state machine этой задачи. `MutexGuard` из `std::sync` не `Send` (намеренно — блокировка ОС-мьютекса привязана к конкретному потоку). Раз `tokio::spawn` требует `Send` для всего Future целиком — компиляция падает **до запуска**, а не превращается в дедлок в рантайме (что было бы типичным поведением при том же паттерне в ручном Boost.Asio-коде: `.lock()` на обычном `std::mutex`, а потом `co_await` — при work-stealing планировщике другой поток может попытаться взять тот же мьютекс, пока текущий держит его и ждёт совсем другого, что реально приводит к дедлокам, которые ловятся только в проде под нагрузкой).

Fix — `tokio::sync::Mutex`, async-aware версия:

```rust
use tokio::sync::Mutex;

async fn good_task(id: u32, data: Arc<Mutex<i32>>) {
    let mut guard = data.lock().await; // .await, не .unwrap()
    sleep(Duration::from_millis(50)).await; // безопасно спать, держа tokio::Mutex
    *guard += 1;
}
```

Прогнал — работает корректно, последовательно: `задача 1 держит лок` → `задача 1 освободила лок` → `задача 2 держит лок` → `задача 2 освободила лок`, итог `2`. `tokio::sync::Mutex::lock()` сам возвращает Future — при конфликте не блокирует ОС-поток целиком, а возвращает `Poll::Pending`, позволяя исполнителю переключиться на другую задачу на этом же потоке, ровно как правильно написанный async-aware мьютекс в связке с Boost.Asio strand'ами.

## `tokio::select!` — гонка futures

```rust
tokio::select! {
    _ = sleep(Duration::from_millis(200)) => {
        println!("сработал первый таймер (200ms)");
    }
    _ = sleep(Duration::from_millis(50)) => {
        println!("сработал второй таймер (50ms) -- он победил");
    }
}
```

Вывод: `сработал второй таймер (50ms) -- он победил`. `select!` ждёт **первую** готовую ветку, остальные — отменяются (об отмене ниже). Ближайший аналог из твоей практики — `boost::asio::experimental::make_parallel_group` (относительно новая фича Boost.Asio) или ручной паттерн с несколькими completion handler'ами и флагом "кто первый" — в tokio это встроенный макрос, а не библиотечная композиция поверх более низкоуровневых примитивов.

## Отмена: `Drop`, а не ручной cancellation token

Вот ещё одно ощутимое эргономическое отличие. В `select!` выше "проигравшая" ветка (200мс таймер) просто **дропается** — Future уничтожается, вызывается `Drop` для всех её полей, и всё. Никакого explicit cancellation protocol не требуется для простого случая. В C++20 отмена suspended coroutine — заметно более ручная штука: нужно вызывать `coroutine_handle::destroy()` явно (RAII-обёртки типа `cppcoro::task` берут это на себя в своём деструкторе, но это осознанная библиотечная работа, не встроенное поведение языка), а для _кооперативной_ отмены (когда код внутри корутины должен узнать, что его хотят остановить, и прибраться) нужен `std::stop_token` (C++20) — отдельный явный механизм, который нужно прокидывать и проверять руками. В Rust отмена через `Drop` — единый механизм с обычным освобождением ресурсов, тот же самый `Drop`, что мы разбирали для `Box`/`Rc`/файлов — Future просто ещё один тип, который дропается как любой другой.

## Async mpsc-канал — прямое продолжение прошлой темы

```rust
let (tx, mut rx) = tokio::sync::mpsc::channel(8); // с ограниченной ёмкостью буфера
tokio::spawn(async move {
    for i in 1..=3 {
        tx.send(i).await.unwrap(); // .await -- если буфер полон, ждём асинхронно
        sleep(Duration::from_millis(20)).await;
    }
});
while let Some(v) = rx.recv().await {
    println!("получено из канала: {v}");
}
```

Вывод: `получено из канала: 1/2/3`. API почти идентичен `std::sync::mpsc` из прошлой темы — та же идиома `Sender`/`Receiver`, только `send`/`recv` теперь `async fn`, не блокирующие поток при ожидании. `tokio::sync::mpsc::channel(8)` — с ограниченным буфером (backpressure "из коробки" — если получатель не успевает, `send().await` сам поставит отправителя в очередь ожидания, не роняя данные и не блокируя поток целиком); есть и `unbounded_channel()` для неограниченного варианта, как у `std::sync::mpsc`.

## Сводная таблица

|                                   | C++20 корутины + Boost.Asio                        | Rust `async`/`await` + tokio                                                          |
| --------------------------------- | -------------------------------------------------- | ------------------------------------------------------------------------------------- |
| Тип корутины                      | Stackless                                          | Stackless                                                                             |
| Кастомизация поведения            | `promise_type` (несколько customization points)    | Трейт `Future` с одним методом `poll` (компилятор генерирует реализацию сам)          |
| Модель резюмирования              | Push: `resume()` продолжает исполнение             | Pull: `poll()` явно отвечает `Ready`/`Pending`                                        |
| Механизм "разбуди меня"           | Completion handler → `resume()`/symmetric transfer | `Waker`, явный объект первого класса                                                  |
| Хранение фрейма                   | Обычно куча (HALO — опциональная оптимизация)      | Стек по умолчанию (гарантия, не оптимизация); куча только при явном стирании типа     |
| Проблема self-referential structs | Решается стабильным адресом кучи                   | Решается типом `Pin<P>`                                                               |
| Исполнитель в стандарте           | Нет (`io_context` — библиотека)                    | Нет (`tokio`/`async-std` — библиотеки)                                                |
| Гонка нескольких операций         | `parallel_group` / ручной паттерн                  | `tokio::select!`                                                                      |
| Отмена                            | Явная (`stop_token`, ручной `destroy()`)           | Неявная, через `Drop`                                                                 |
| Мьютекс, безопасный для `.await`  | Strand или ручная синхронизация                    | `tokio::sync::Mutex` (компилятор ловит неправильное использование `std::sync::Mutex`) |
| Каналы                            | Нет в стандарте, ручные/сторонние                  | `tokio::sync::mpsc`/`oneshot`/`broadcast`/`watch`                                     |
| Многопоточный планировщик задач   | Ручная настройка `io_context` + `strand`           | Встроенный work-stealing в `tokio` (multi-thread flavor)                              |

Главный практический вывод для тебя: то, что ты уже интуитивно понимаешь про stackless-корутины из C++20 (frame, suspension points, resumption), переносится почти один в один — просто интерфейс `poll`/`Waker` явнее, чем `await_ready`/`await_suspend`/`await_resume`, а компилятор дополнительно ловит на этапе компиляции целый класс багов (не-`Send` состояние через `.await`, use-after-move в замыканиях задач), которые в мире Boost.Asio ты бы отловил либо ревью кода, либо TSan, либо продакшен-инцидентом.
