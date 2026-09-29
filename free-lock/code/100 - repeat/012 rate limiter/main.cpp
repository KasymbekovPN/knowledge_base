
// ============================================================
// ЗАДАЧА: Rate Limiter (Token Bucket)
//
// Реализовать структуру данных, ограничивающую частоту событий
// по алгоритму token bucket: "ведро" токенов пополняется с
// фиксированной скоростью, каждое событие потребляет один токен;
// если токенов нет -- событие отклоняется.
//
// Типичный сценарий: ограничение частоты API-запросов от клиента,
// троттлинг, защита backend'а от всплесков трафика, при этом
// позволяя кратковременные "всплески" (burst) в пределах ёмкости
// ведра -- в отличие от простого fixed-window счётчика.
//
// Минимальный контракт:
//   - try_acquire()  -- попытаться получить один токен на событие;
//                        true, если разрешено (токен списан),
//                        false, если лимит превышен
//
// ВСЁ ОСТАЛЬНОЕ -- открытые вопросы. Прежде чем писать тесты и
// реализацию, стоит их прояснить, а не додумывать самостоятельно.
// Некоторые направления, где условие намеренно неполно:
//   - Как параметризуется ведро -- ёмкость (максимум токенов) и
//     скорость пополнения (токенов в секунду) как ДВА независимых
//     параметра, или один параметр задаёт оба неявно?
//   - Ведро стартует ПОЛНЫМ (можно сразу же "сжечь" burst) или
//     ПУСТЫМ (нужно сначала накопить токены)?
//   - Пополнение непрерывное (дробные токены накапливаются
//     постоянно) или дискретное (целые токены раз в тик)?
//   - Нужна ли возможность запросить БОЛЬШЕ одного токена за раз
//     (для "дорогих" операций, например try_acquire(5)), или
//     всегда ровно один токен на вызов?
//   - Что если запрошено больше токенов, чем вообще ёмкость ведра
//     -- всегда false, или это отдельная ошибка?
//   - Это ОДНО ведро на весь лимитер, или РАЗНЫЕ ведра на ключ
//     (например, отдельный лимит на каждого клиента по его ID,
//     как в TTLCounter из прошлого упражнения)?
//   - try_acquire() должен вернуть просто bool, или что-то более
//     информативное (например, сколько токенов сейчас доступно,
//     или через сколько мс появится следующий токен)?
//   - Нужна ли потокобезопасность? Если да -- одна блокировка на
//     всю структуру, или более тонкая гранулярность (например,
//     атомарный счётчик токенов без мьютекса)?
//   - Источник времени -- реальные часы или подставной clock для
//     детерминированных тестов (актуально, как и в TTLCounter --
//     тесты на время склонны быть хрупкими на реальном sleep_for)?
// ============================================================

#include <string>
#include <iostream>
#include <cassert>
#include <chrono>
#include <thread>
#include <atomic>
#include <vector>

// ------------------------------------------------------------
// TODO: реализация класса RateLimiter
// ------------------------------------------------------------

namespace {
    class RateLimiter {
        static constexpr int DEFAULT_MAX_TOKEN_QUANTITY{100};

        using time_primitive_t = std::chrono::steady_clock::rep;

        std::atomic<int> down_counter_;
        std::atomic<time_primitive_t> last_timestamp_;
        int max_token_quantity;
        time_primitive_t time_window_step_;


    public:
        explicit RateLimiter(const int max_token_quantity, const std::chrono::milliseconds time_window):
            max_token_quantity{max_token_quantity},
            time_window_step_(time_window.count() * 1'000'000 / max_token_quantity) {

            down_counter_.store(
                max_token_quantity > 0 ? max_token_quantity : DEFAULT_MAX_TOKEN_QUANTITY,
                std::memory_order_relaxed
            );
            last_timestamp_.store(0, std::memory_order_relaxed);
        }

        bool try_acquire() {
            auto prev_last_timestamp{last_timestamp_.load(std::memory_order_relaxed)};
            const auto new_last_timestamp = std::chrono::steady_clock::now().time_since_epoch().count();

            if (int dif{static_cast<int>((new_last_timestamp - prev_last_timestamp) / time_window_step_)};
                dif) {
                while (!last_timestamp_.compare_exchange_weak(
                prev_last_timestamp,
                    new_last_timestamp,
                    std::memory_order_acq_rel,
                    std::memory_order_relaxed)) {
                        dif = static_cast<int>((new_last_timestamp - prev_last_timestamp) / time_window_step_);
                    }

                    if (prev_last_timestamp != 0 && dif != 0) {
                        auto old_dc{down_counter_.load(std::memory_order_relaxed)};
                        while (!down_counter_.compare_exchange_weak(
                            old_dc,
                            std::min(old_dc + dif, max_token_quantity),
                            std::memory_order_acq_rel,
                            std::memory_order_relaxed)) {}
                    }
            }

            auto prev_dc{down_counter_.load(std::memory_order_acquire)};

            if (prev_dc == 0) return false;

            while (!down_counter_.compare_exchange_weak(
                prev_dc,
                prev_dc - 1,
                std::memory_order_acq_rel,
                std::memory_order_relaxed)) {

                if (prev_dc == 0) return false;
            }

            return true;
        }
    };
}

// ------------------------------------------------------------
// Тесты -- содержимое закомментировано, т.к. реализации ещё нет.
// Раскомментировать и адаптировать по мере того, как контракт
// проясняется и реализация появляется.
// ------------------------------------------------------------

void test_basic_acquire_within_capacity() {
    std::cout << "[test_basic_acquire_within_capacity] ";

    RateLimiter limiter{5, std::chrono::milliseconds{100}};
    for (int i{}; i < 5; ++i) {
        assert(limiter.try_acquire() == true);
    }
    std::cout << "TODO\n";
}

void test_acquire_exceeds_capacity() {
    std::cout << "[test_acquire_exceeds_capacity] ";
    // Зависит от того, стартует ли ведро полным -- см. открытый
    // вопрос в описании задачи.

    RateLimiter limiter{3, std::chrono::milliseconds{100}};
    for (int i = 0; i < 3; ++i) {
        assert(limiter.try_acquire() == true);
    }
    assert(limiter.try_acquire() == false); // ведро пусто
    std::cout << "TODO\n";
}

void test_refill_over_time() {
    std::cout << "[test_refill_over_time] ";
    // Понадобится способ управлять "временем" в тесте -- либо
    // подставной clock, либо sleep_for (менее надёжно, подвержено
    // таймингам, как и в TTLCounter).

    RateLimiter limiter{2, std::chrono::milliseconds{100}};
    assert(limiter.try_acquire() == true);
    assert(limiter.try_acquire() == true);
    assert(limiter.try_acquire() == false); // ведро пусто
    std::this_thread::sleep_for(std::chrono::milliseconds(110)); // > 100мс на токен
    assert(limiter.try_acquire() == true); // токен успел появиться
    std::cout << "TODO\n";
}

void test_refill_does_not_exceed_capacity() {
    std::cout << "[test_refill_does_not_exceed_capacity] ";
    // Долгое бездействие НЕ должно позволить накопить токенов
    // больше ёмкости ведра -- иначе это уже не token bucket,
    // а просто счётчик с задержкой.

    RateLimiter limiter{3, std::chrono::milliseconds{100}};
    std::this_thread::sleep_for(std::chrono::seconds(2)); // ждём ОЧЕНЬ долго
    int success_acquired = 0;
    while (limiter.try_acquire()) ++success_acquired;
    assert(success_acquired == 3); // не больше ёмкости, сколько бы времени ни прошло
    std::cout << "TODO\n";
}

void test_concurrent_acquire() {
    std::cout << "[test_concurrent_acquire] ";

    RateLimiter limiter{3, std::chrono::milliseconds{100}};

    std::atomic<int> counter{0};
    std::vector<std::thread> threads;
    threads.reserve(3);

    for (int p{}; p < 3; ++p) {
        threads.emplace_back([&counter, &limiter] {
            for (int i{}; i < 5; ++i) {
                if (limiter.try_acquire()) {
                    counter.fetch_add(1, std::memory_order_relaxed);
                }
            }
        });
    }

    for (auto& t: threads) t.join();

    std::cout << "TODO\n";
}

void test_bucket_overflow() {
    std::cout << "[test_bucket_overflow] ";

    RateLimiter limiter{3, std::chrono::milliseconds{100}};

    int success_acquired = 0;
    while (limiter.try_acquire()) ++success_acquired;
    assert(success_acquired == 3); // не больше ёмкости, сколько бы времени ни прошло

    std::this_thread::sleep_for(std::chrono::seconds(1));

    while (limiter.try_acquire()) ++success_acquired;
    assert(success_acquired == 6); // не больше ёмкости, сколько бы времени ни прошло

    std::cout << "TODO\n";
}

// ------------------------------------------------------------
// main
// ------------------------------------------------------------

int main() {
    test_basic_acquire_within_capacity();
    test_acquire_exceeds_capacity();
    test_refill_over_time();
    test_refill_does_not_exceed_capacity();
    test_concurrent_acquire();
    test_bucket_overflow();

    std::cout << "\nВсе тестовые методы вызваны (содержимое ещё не реализовано).\n";
    return 0;
}