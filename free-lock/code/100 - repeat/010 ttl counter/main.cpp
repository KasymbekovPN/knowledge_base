// ============================================================
// ЗАДАЧА: TTL Counter
//
// Реализовать структуру данных, которая считает события по ключу
// (строка), где каждая запись "живёт" ограниченное время (TTL) --
// по истечении этого времени запись перестаёт учитываться.
//
// Типичный сценарий: rate limiting ("не больше N запросов за
// последние T секунд"), метрики активности за скользящее окно.
//
// Минимальный контракт:
//   - increment(key)  -- зарегистрировать одно событие по ключу
//   - get(key)         -- вернуть текущий счётчик по ключу
//
// ВСЁ ОСТАЛЬНОЕ -- открытые вопросы. Прежде чем писать тесты и
// реализацию, стоит их прояснить (задать вопросы, как в реальной
// задаче/тикете), а не додумывать самостоятельно. Некоторые
// направления, где условие намеренно неполно:
//   - Что значит "TTL" для конкретной записи -- отсчитывается от
//     первого increment по ключу, от последнего, или у каждого
//     отдельного события свой независимый TTL (скользящее окно)?
//   - increment() по уже существующему ключу -- продлевает TTL
//     (sliding expiration) или нет?
//   - get() на истёкший/несуществующий ключ -- 0? std::optional?
//     исключение?
//   - Вытеснение истёкших записей -- лениво (только при обращении)
//     или активно (фоновый поток/периодическая чистка)?
//   - Нужна ли потокобезопасность? Если да -- одна блокировка на
//     всю структуру, или более тонкая гранулярность?
//   - Точность времени -- секунды/миллисекунды? Какой источник
//     времени (реальные часы vs подставной clock для тестов)?
//   - Есть ли верхняя граница на число ключей (аналог capacity
//     в LRU), или структура растёт неограниченно?
// ============================================================

#include <unordered_map>
#include <string>
#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>
#include <thread>
#include <cassert>

// ------------------------------------------------------------
// TODO: реализация класса TTLCounter
// ------------------------------------------------------------

namespace {
    class TTLCounter {
        static inline std::chrono::milliseconds DEFAULT_TTL{1000};

        std::mutex mutex_;
        std::chrono::milliseconds ttl_;
        std::unordered_map<std::string, std::vector<std::chrono::steady_clock::time_point>> counters_;

        [[nodiscard]] std::chrono::steady_clock::time_point get_now() const {
            return std::chrono::steady_clock::now();
        }

        // call under mutex_ in write semantic
        void clean_unsafe(const std::string& key, const std::chrono::steady_clock::time_point& now) {
            const auto it{counters_.find(key)};
            if (it == counters_.end()) return;

            auto& vec{it->second};
            const auto first_valid{std::find_if(vec.begin(), vec.end(), [&] (const auto& tp) {
                return (now - tp) <= ttl_;
            })};
            vec.erase(vec.begin(), first_valid);
        }

    public:
        explicit TTLCounter(const std::chrono::milliseconds ttl = DEFAULT_TTL):
            ttl_{ttl > std::chrono::milliseconds(0) ? ttl : DEFAULT_TTL} {}

        // TODO: increment(key) -- зарегистрировать событие
        void increment(const std::string& key) {
            const auto now{get_now()};
            std::lock_guard<std::mutex> lock{mutex_};
            clean_unsafe(key, now);

            counters_[key].push_back(now);
            // if (counters_.contains(key)) {
            //     counters_[key].push_back(now);
            // } else {
            //     counters_.emplace(key, std::vector<std::chrono::steady_clock::time_point>{now});
            // }
        }

        // TODO: get(key) -- вернуть текущий счётчик
        [[nodiscard]] int get(const std::string& key) {
            int result{};
            {
                const auto now{get_now()};
                std::lock_guard<std::mutex> lock{mutex_};
                clean_unsafe(key, now);

                if (counters_.contains(key)) {
                    result = static_cast<int>(counters_[key].size());
                }
            }

            return result;
        }
    };
}

// ------------------------------------------------------------
// Тесты -- содержимое закомментировано, т.к. реализации ещё нет.
// Раскомментировать и адаптировать по мере того, как контракт
// проясняется и реализация появляется.
// ------------------------------------------------------------

void test_basic_increment_and_get() {
    std::cout << "[test_basic_increment_and_get] ";
    TTLCounter counter{};
    counter.increment("a");
    counter.increment("a");
    counter.increment("a");
    assert(counter.get("a") == 3);
    std::cout << "TODO\n";
}

void test_nonexistent_key() {
    std::cout << "[test_nonexistent_key] ";
    TTLCounter counter{};
    assert(counter.get("missing") == 0); // или std::optional<int>{} -- уточнить контракт
    std::cout << "TODO\n";
}

void test_ttl_expiration() {
    std::cout << "[test_ttl_expiration] ";
    // Понадобится способ управлять "временем" в тесте --
    // либо подставной clock, либо sleep_for (менее надёжно,
    // подвержено таймингам в CI).

    TTLCounter counter{std::chrono::milliseconds(100)};
    counter.increment("a");
    assert(counter.get("a") == 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    assert(counter.get("a") == 0); // запись истекла
    std::cout << "TODO\n";
}

void test_increment_resets_or_not_ttl() {
    std::cout << "[test_increment_resets_or_not_ttl] ";
    // Зависит от того, sliding expiration или нет -- нужно сначала
    // прояснить контракт, прежде чем этот тест вообще имеет смысл
    // писать в конкретном виде.
    //
    TTLCounter counter(std::chrono::milliseconds(100));
    counter.increment("a");
    std::this_thread::sleep_for(std::chrono::milliseconds(60));
    counter.increment("a");
    std::this_thread::sleep_for(std::chrono::milliseconds(60));
    assert(counter.get("a") == 1);
    std::cout << "TODO\n";
}

void test_multiple_keys_independent() {
    std::cout << "[test_multiple_keys_independent] ";
    TTLCounter counter{};
    counter.increment("a");
    counter.increment("b");
    counter.increment("b");
    assert(counter.get("a") == 1);
    assert(counter.get("b") == 2);
    std::cout << "TODO\n";
}

void test_zero_or_edge_ttl() {
    std::cout << "[test_zero_or_edge_ttl] ";

    TTLCounter counter{std::chrono::milliseconds(0)};
    counter.increment("a");
    counter.increment("a");
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    assert(counter.get("a") == 2);

    counter.increment("a");
    counter.increment("a");
    counter.increment("a");
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    assert(counter.get("a") == 5);

    counter.increment("a");
    counter.increment("a");
    counter.increment("a");
    counter.increment("a");
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    assert(counter.get("a") == 7);

    std::cout << "TODO\n";
}

// ------------------------------------------------------------
// main
// ------------------------------------------------------------

int main() {
    test_basic_increment_and_get();
    test_nonexistent_key();
    test_ttl_expiration();
    test_increment_resets_or_not_ttl();
    test_multiple_keys_independent();
    test_zero_or_edge_ttl();

    std::cout << "\nAll called.\n";
    return 0;
}