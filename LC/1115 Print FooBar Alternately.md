
## Суть задачи

Два потока: один вызывает `foo()`, другой — `bar()`. Нужно, чтобы они печатали строго чередуясь: `foo bar foo bar foo bar ...` (n раз каждый), в сумме давая `foobarfoobarfoobar...`. В отличие от LC1114, здесь не "разовая" последовательность из трёх шагов, а **циклическое чередование** n раз — то есть после `bar` снова должен идти `foo`, а не завершение.

## Идея

Нужен флаг "чья сейчас очередь" (`turn`), и **оба** потока должны уметь ждать друг друга — не как в LC1114, где было одностороннее ожидание (second ждёт first, third ждёт second), а взаимное: `foo` ждёт, пока не освободится `bar`, и наоборот, в цикле.

Ключевое отличие от LC1114: там был **один** линейный флаг состояния (`0 → 1 → 2`), который двигался только вперёд. Здесь флаг **осциллирует** между двумя состояниями внутри цикла — по сути, это простейший пример паттерна "ping-pong" синхронизации, который лежит в основе многих producer-consumer-подобных схем.

Несмотря на то, что поток `t1` (bar) запущен раньше `t2` (foo), он **обязан** дождаться `fooTurn == true` прежде чем напечатать что-либо — начальное значение `fooTurn = true` гарантирует, что первым реально напечатается `foo`, независимо от порядка старта потоков.

## Что важно проговорить на собеседовании

**1. Почему `notify_all()`, а не `notify_one()` — здесь это не просто "на всякий случай", а вопрос корректности при масштабировании задачи.** Если бы задача требовала несколько потоков `foo` и несколько потоков `bar` (вариация, которую иногда дают как follow-up), `notify_one()` может разбудить "не того" — например, ещё один поток `bar`, который снова заснёт, увидев `fooTurn == false`, в то время как реальный ожидающий `foo`-поток пропустит побудку. `notify_all()` будит всех, и предикат в `wait` сам отфильтровывает, кому действительно пора работать — это чуть менее эффективно (лишние пробуждения), но корректно в общем случае.

**2. Альтернатива через два семафора — более "чистое" решение для строгого ping-pong, и часто ожидается как более продвинутый ответ:**

Это концептуально чище, чем mutex+condvar+bool-флаг, потому что семафор **сам является** счётчиком разрешений — не нужен отдельный "флаг состояния", который вручную защищается mutex'ом. Стоит явно назвать этот вариант на собеседовании как альтернативу — это показывает, что вы знаете более широкий набор примитивов синхронизации, чем просто condition_variable, а именно это могут спросить как "а какие ещё есть способы?".

**3. Связь с реальными системами.** Строгое чередование двух потоков — редкий паттерн сам по себе в продакшене (обычно нужно наоборот — максимальный параллелизм), но сама механика "взаимно сигнализирующих состояний" — фундамент для **double buffering** (два потока попеременно пишут в буфер A / читают из буфера B, потом меняются местами) — паттерн, который реально используется в пайплайнах обработки потоковых данных, что прямая параллель с "рантаймом как графом стадий исполнения" из вашей вакансии: одна стадия не может начать читать, пока предыдущая не закончила писать в тот же буфер, и наоборот для следующего цикла.

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <semaphore>

namespace {
    class FooBarBlocked {
        static constexpr int MIN_N{3};

        std::mutex mtx;
        std::condition_variable cv;
        int n;
        bool foo_turn;

        public:
            explicit FooBarBlocked(const int n):
                n{n <= 0 ? MIN_N : n},
                foo_turn{true} {}

        void foo(const std::function<void()>& print_foo) {
            for (int i{}; i < n; ++i) {
                std::unique_lock<std::mutex> lk(mtx);
                cv.wait(lk, [this] { return foo_turn; });
                print_foo();
                foo_turn = false;
                cv.notify_all();
            }
        }

        void bar(const std::function<void()>& print_bar) {
            for (int i{}; i < n; ++i) {
                std::unique_lock<std::mutex> lk(mtx);
                cv.wait(lk, [this] { return !foo_turn; });
                print_bar();
                foo_turn = true;
                cv.notify_all();
            }
        }
    };

    class FooBarSemaphore {
        static constexpr int MIN_N{3};

        std::binary_semaphore foo_sem{1};
        std::binary_semaphore bar_sem{0};
        int n;

        public:
            explicit FooBarSemaphore(const int n): n{n <= 0 ? MIN_N : n} {}

            void foo(const std::function<void()>& print_foo) {
                for (int i{}; i < n; ++i) {
                    foo_sem.acquire();
                    print_foo();
                    bar_sem.release();
                }
            }

            void bar(const std::function<void()>& print_bar) {
                for (int i{}; i < n; ++i) {
                    bar_sem.acquire();
                    print_bar();
                    foo_sem.release();
                }
            }
    };

    void start_test0() {
        const auto print_foo = [] { std::cout << "foo "; };
        const auto print_bar = [] { std::cout << "bar\n"; };

        FooBarBlocked foo_bar{5};

        std::thread t1{&FooBarBlocked::bar, &foo_bar, print_bar};
        std::thread t2{&FooBarBlocked::foo, &foo_bar, print_foo};

        t1.join();
        t2.join();
    }

    void start_test1() {
        const auto print_foo = [] { std::cout << "foo "; };
        const auto print_bar = [] { std::cout << "bar\n"; };

        FooBarSemaphore foo_bar{5};

        std::thread t1{&FooBarSemaphore::bar, &foo_bar, print_bar};
        std::thread t2{&FooBarSemaphore::foo, &foo_bar, print_foo};

        t1.join();
        t2.join();
    }

}

int main() {
    std::cout << "### start_test0 ###\n";
    start_test0();

    std::cout << "### start_test1 ###\n";
    start_test1();

    return 0;
}

```