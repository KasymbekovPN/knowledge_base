
## Суть задачи

Три метода — `first()`, `second()`, `third()` — вызываются в **трёх разных потоках**, и порядок запуска потоков непредсказуем (гарантий на порядок вызова методов нет). Нужно обеспечить, чтобы вывод всегда был `first → second → third`, независимо от того, в каком порядке ОС решит запустить потоки.

Это первая, самая базовая задача из concurrency-раздела LeetCode — по сути, "hello world" для примитивов синхронизации.

## Идея

Нужны два "сигнала":

- "first завершился" → можно запускать second
- "second завершился" → можно запускать third

Классический инструмент для "подождать, пока не наступит условие" — `std::condition_variable` + `std::mutex` + флаг состояния (сам mutex не хранит состояния, флаг обязателен, чтобы не терять сигналы и защититься от spurious wakeup).

**Почему предикат в `cv.wait`, а не просто `cv.wait(lock)`:** condition_variable подвержена **spurious wakeup** — поток может проснуться без реального `notify`. Форма `wait(lock, predicate)` — это short-hand для `while (!predicate()) wait(lock);`, что защищает от ложных пробуждений _и_ от гонки "notify пришёл до того, как слушатель успел войти в wait" (потому что предикат перепроверяется, а не полагается только на факт пробуждения).

**Почему `notify_all()`, а не `notify_one()`:** здесь не критично (только один поток реально ждёт нужного состояния), но `notify_all` безопаснее по умолчанию, когда несколько потоков потенциально могут ждать на одной cv с разными предикатами — иначе можно разбудить "не того" ждущего, а нужный так и останется спать.

## Что важно проговорить на собеседовании

- **Это не lock-free** — здесь классическая блокирующая синхронизация (mutex + condvar), причём в вашем описании профиля вы сами отметили это как более привычный вам подход. Стоит явно понимать разницу: `mutex` + `condition_variable` — это **primitive для взаимного исключения и ожидания события**, тогда как lock-free структуры избегают блокировок вовсе, используя atomic compare-and-swap.
- **Альтернатива без condition_variable — через atomics и busy-wait**, что ближе к lock-free-духу (хотя формально это ещё не lock-free, а spin-wait):

Здесь стоит явно объяснить выбор `memory_order_release`/`memory_order_acquire`: `release` на записи гарантирует, что всё, что было записано _до_ него (в данном случае — сам вызов `printFirst()`), не будет переупорядочено компилятором/CPU так, чтобы оказаться _после_ этой записи с точки зрения другого потока. `acquire` на чтении даёт симметричную гарантию — если поток увидел значение через acquire-load, то он гарантированно видит и всё, что было записано _до_ соответствующего release-store в другом потоке. Без этого (например, с `memory_order_relaxed`) — компилятор/CPU формально имеют право переупорядочить операции так, что `second()` начнёт выполняться до того, как эффекты `first()` (включая сам вывод в консоль, если бы там было что-то, зависящее от памяти) станут видны — на практике для `cout` это маловероятно сломается на x86 из-за сильной модели памяти, но на ARM это уже реальный риск, и на собеседовании явное упоминание acquire/release — как раз то самое "объяснить memory ordering", что было в списке ожидаемых вопросов.

- **Busy-wait жжёт CPU** — в отличие от `condition_variable`, где ждущий поток реально **спит** (не потребляет CPU-время), spin-wait версия крутит цикл, потребляя 100% ядра, пока ждёт. Это осознанный trade-off: spin-wait оправдан, только если ожидаемое время блокировки **очень короткое** (наносекунды-микросекунды) — иначе `condition_variable` почти всегда правильнее.

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

namespace {

    class FLocked {
        std::mutex mtx;
        std::condition_variable cv;
        int state;

    public:
        explicit FLocked(): state{0} {}

        void first(const std::function<void()>& print_first) {
            std::unique_lock<std::mutex> lk(mtx);
            print_first();
            state = 1;
            cv.notify_all();
        }

        void second(const std::function<void()>& print_second) {
            std::unique_lock<std::mutex> lk(mtx);
            cv.wait(lk, [this]{ return state == 1; });
            print_second();
            state = 2;
            cv.notify_all();
        }

        void third(const std::function<void()>& print_third) {
            std::unique_lock<std::mutex> lk(mtx);
            cv.wait(lk, [this]{ return state == 2; });
            print_third();
        }
    };

    class FLockFree {
        std::atomic<int> state{0};

    public:
        void first(const std::function<void()>& print_first) {
            print_first();
            state.store(1, std::memory_order_release);
        }

        void second(const std::function<void()>& print_second) {
            while (state.load(std::memory_order_acquire) != 1) {}
            print_second();
            state.store(2, std::memory_order_release);
        }

        void third(const std::function<void()>& print_third) const {
            while (state.load(std::memory_order_acquire) != 2) {}
            print_third();
        }
    };

    void start_test30() {
        const auto print_first = [] { std::cout << "F30 first\n"; };
        const auto print_second = [] { std::cout << "F30 second\n"; };
        const auto print_third = [] { std::cout << "F30 third\n"; };

        FLocked f;
        std::thread t1{&FLocked::first, &f, print_first};
        std::thread t2{&FLocked::second, &f, print_second};
        std::thread t3{&FLocked::third, &f, print_third};

        t3.join();
        t1.join();
        t2.join();
    }

    void start_test31() {
        const auto print_first = [] { std::cout << "F31 first\n"; };
        const auto print_second = [] { std::cout << "F31 second\n"; };
        const auto print_third = [] { std::cout << "F31 third\n"; };

        FLockFree f;
        std::thread t1{&FLockFree::first, &f, print_first};
        std::thread t2{&FLockFree::second, &f, print_second};
        std::thread t3{&FLockFree::third, &f, print_third};

        t3.join();
        t1.join();
        t2.join();
    }
}

int main() {
    start_test30();
    start_test31();

    return 0;
}

```
