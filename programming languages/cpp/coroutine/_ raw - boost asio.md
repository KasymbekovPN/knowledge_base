---
tags:
  - programming-language
  - cpp
  - coroutine
---
[[programming languages/cpp/coroutine/_|<=]]

Покажу тот же сценарий на Boost.Asio. Но честно оговорюсь сразу: в этом окружении нет сети и нет установленного Boost, поэтому я не смогу скомпилировать и запустить пример прямо здесь, как делал раньше. Дам выверенный код и подробно объясню соответствие с нашим самодельным планировщиком — собрать его ты сможешь у себя.

Сначала проверю окружение, вдруг Boost всё же есть.Boost не установлен, а сети нет — поставить не выйду. Поэтому код привожу как выверенный листинг (он корректен для Boost 1.80+), но без прогона. Разберу его тщательно, чтобы соответствие с нашим планировщиком было ясным.

## Тот же сценарий на Boost.Asio

Воспроизвожу `ticker` (несколько задач с таймерами) и `latecomer` (задача, подкинутая «извне»):

```cpp
// asio_scheduler.cpp
// Сборка (header-only Asio): g++ -std=c++20 -O2 asio_scheduler.cpp -o asio_sched -pthread
//   с Boost:                 g++ -std=c++20 -O2 asio_scheduler.cpp -o asio_sched -pthread -lboost_system
// Запуск: ./asio_sched
//
#include <boost/asio.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <chrono>
#include <iostream>
#include <string>

namespace asio = boost::asio;
using namespace std::chrono_literals;

// awaitable<void> — это "Task" из Asio: тип возврата корутины
asio::awaitable<void> ticker(std::string name, int ticks,
                             std::chrono::milliseconds period) {
    // получаем executor текущей корутины (кто нас исполняет)
    auto ex = co_await asio::this_coro::executor;

    for (int i = 1; i <= ticks; ++i) {
        std::cout << "  " << name << " tick " << i << '\n';

        // аналог нашего co_await sched.sleep_for(period):
        asio::steady_timer timer(ex, period);
        co_await timer.async_wait(asio::use_awaitable);   // приостановка по таймеру
    }
    std::cout << "  " << name << " finished\n";
}

asio::awaitable<void> latecomer() {
    std::cout << ">>> latecomer ran!\n";
    co_return;
}

int main() {
    asio::io_context io;          // <-- "взрослый" аналог нашего Scheduler

    // запускаем задачи на io_context (аналог sched.schedule(...))
    asio::co_spawn(io, ticker("fast", 3, 50ms),  asio::detached);
    asio::co_spawn(io, ticker("slow", 2, 120ms), asio::detached);

    // "внешнее" событие через 30мс, не блокируя поток:
    // таймер сам подкинет latecomer в очередь io_context
    asio::steady_timer late(io, 30ms);
    late.async_wait([&io](auto /*ec*/) {
        std::cout << "[timer] injecting latecomer\n";
        asio::co_spawn(io, latecomer(), asio::detached);
    });

    io.run();                     // <-- "взрослый" аналог нашего sched.run()
    std::cout << "done\n";
    return 0;
}
```

## Соответствие нашему планировщику — построчно

Вся наша ручная машинерия имеет прямой аналог в Asio. Это и есть главная ценность сравнения:

|Наш планировщик|Boost.Asio|Что делает|
|---|---|---|
|`class Scheduler`|`asio::io_context`|хранит очередь готовых задач и event loop|
|`sched.run()`|`io.run()`|крутит loop, пока есть работа|
|`Task` (наш тип-обёртка)|`asio::awaitable<T>`|тип возврата корутины|
|`sched.schedule(h)`|`asio::co_spawn(io, coro, ...)`|запустить корутину на loop|
|`co_await sched.sleep_for(d)`|`co_await timer.async_wait(use_awaitable)`|приостановка по таймеру|
|`SleepAwaiter` + `priority_queue` таймеров|`asio::steady_timer`|механика отложенного возобновления|
|ручной `promise_type`, `coroutine_handle`|спрятаны внутри `awaitable`/`co_spawn`|—|

То есть `io_context` — это наш `Scheduler`, доведённый до промышленного уровня; `awaitable<T>` — наш `Task` с правильным `promise_type`; `co_spawn` — наш `schedule` плюс корректное управление временем жизни корутины.

## Ключевое отличие — чего мы не смогли сделать руками

Вспомни главный нерешённый пробел нашего `run()`: он умел ждать только таймеры и внешние пробуждения через `condition_variable`, но **не нативное I/O**. Вот здесь Asio принципиально мощнее.

`io.run()` внутри блокируется не на `sleep_until` и не на `condition_variable`, а на нативном механизме ОС — `epoll` (Linux), `kqueue` (BSD/macOS), IOCP (Windows). Этот механизм ждёт **одновременно** таймеры _и_ готовность сокетов/файлов, просыпаясь от любого события. Именно то, что я описывал как «взрослый» loop в разборе `sleep_until`. Поэтому в Asio `co_await socket.async_read_some(...)` работает так же естественно, как `co_await timer.async_wait(...)` — а у нас I/O вообще не было.

## Про «latecomer» и пробуждение loop

В нашей `condition_variable`-версии мы городили потокобезопасную очередь + `notify_one`, чтобы разбудить спящий loop внешним событием. В Asio это получается само:

- Если событие приходит **в том же потоке** — `async_wait` с колбэком просто кладёт продолжение в очередь `io_context`, и loop его подхватит. Здесь я использовал таймер на 30мс как «источник» события (сети-то нет), но это мог бы быть готовый сокет.
- Если нужно подкинуть работу **из другого потока** — используется `asio::post(io, ...)` или `asio::dispatch`, которые потокобезопасно добавляют задачу и будят loop. Это прямой аналог нашего `schedule()` + `notify_one()`, только корректный и кроссплатформенный из коробки.

```cpp
// аналог нашего внешнего потока, будящего loop:
std::thread external([&io]{
    std::this_thread::sleep_for(50ms);
    asio::post(io, []{ /* ... или co_spawn ... */ });   // потокобезопасно + разбудит run()
});
```

## Многопоточность — бесплатно

Помнишь, как наш переход к `condition_variable` + mutex был первым шагом к пулу потоков? В Asio это буквально несколько строк: один `io_context`, и `run()` вызывается из нескольких потоков:

```cpp
asio::io_context io;
// ... co_spawn задач ...
std::vector<std::thread> pool;
for (int i = 0; i < 4; ++i)
    pool.emplace_back([&io]{ io.run(); });   // 4 потока крутят один loop
for (auto& t : pool) t.join();
```

Теперь корутины распределяются по 4 потокам — настоящий параллелизм (пункт I/O vs CPU). Для защиты от гонок при этом используют `asio::strand` — он сериализует обработчики конкретного объекта, не блокируя весь пул. Всё то, что нам пришлось бы писать вручную и очень аккуратно.

## Что брать из этого сравнения

Наш самодельный планировщик был полезен как _модель_: он показал, что `io_context` — не магия, а та же очередь готовых корутин + механизм ожидания, что мы собрали сами. Но для реального кода писать это руками не нужно и опасно (легко словить UB на времени жизни, гонки, забытый `destroy`). Asio даёт:

- готовый, выверенный event loop с нативным I/O (`epoll`/IOCP);
- `awaitable<T>` вместо ручного `Task`/`promise_type`;
- корректное управление временем жизни через `co_spawn`;
- кроссплатформенность и многопоточность из коробки.

Наш код отвечал на вопрос «_как это устроено_», Asio отвечает на «_что брать в продакшен_».

Поскольку запустить здесь не могу — если соберёшь у себя, ожидаемый вывод будет вида: тики `fast` и `slow` чередуются по своим интервалам (fast в ~2.4 раза чаще), а `latecomer` впечатается примерно на 30мс, не нарушая хода таймеров. Хочешь, вместо Asio покажу тот же сценарий на **cppcoro** (он ближе к нашему `Task` по духу — там есть `task<T>`, `static_thread_pool`, `when_all`), или разберём этап 7 — ловушки времени жизни в корутинах, которые в этом Asio-коде как раз легко словить с захватами в лямбдах?


---
---
---






## Этап 6. Реальное применение

- Асинхронный сетевой сервер (эхо-сервер на Asio с корутинами)
- Конвейер обработки данных через генераторы
- Комбинирование с пулом потоков (несколько потоков × тысячи корутин)

## Этап 7. Подводные камни

- Висячие ссылки: захват по ссылке в корутине, переживший вызывающую сторону (особенно с лямбдами)
- Время жизни объектов и аргументов относительно coroutine frame
- Аллокации и попытки их избежать (Heap Allocation Elision Optimization, HALO)
- Отладка: почему стек-трейсы корутин «непривычны»

---

## Полезные ресурсы

- Серия статей **Lewis Baker «Asymmetric Transfer»** — лучший разбор внутренностей
- Доклады **CppCon** по корутинам (особенно вводные от Lewis Baker и Gor Nishanov)
- **cppreference** — раздел Coroutines как справочник
- Книга **«C++ Concurrency in Action» (Anthony Williams)**, 2-е издание — есть глава по корутинам

---

**Совет по темпу:** этапы 2–3 самые тяжёлые и неинтуитивные — не пытайся проскочить их быстро. Сразу пиши код руками: понимание `promise_type` и awaiter приходит только через собственные реализации, а не через чтение. Этапы 5–6 идут гораздо легче, когда фундамент заложен.

Хочешь, оформлю это в виде файла (Markdown), чтобы можно было отмечать прогресс по пунктам?