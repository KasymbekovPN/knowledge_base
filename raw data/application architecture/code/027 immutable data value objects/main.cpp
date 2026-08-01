// Immutable value objects: почему невозможность мутации сама по себе
// устраняет целый класс багов - aliasing (два держателя одного объекта,
// один меняет, оба видят изменение) и гонки данных между потоками.

#include <iostream>
#include <format>
#include <thread>
#include <vector>

// ============================================================
// Вариант 1: мутабельный Money - классический aliasing-баг
// ============================================================
namespace mutable_version {
    namespace {
        class Money {
        public:
            explicit Money(const long cents): cents_(cents) {}
            // МУТИРУЕТ объект на месте
            void add(const long cents) { cents_ += cents; }
            long cents() const { return cents_; }
        private:
            long cents_;
        };
    }

    static void demoAliasing() {
        std::cout << "-- mutable version: aliasing bug --\n";
        Money price{1'000};
        // кто-то держит ссылку на тот же объект
        Money& alias = price;
        // и ещё указатель на него же
        const Money* pointer_holder = &price;

        // "локальное" изменение через alias...
        alias.add(500);
        std::cout << "  price.cents()           = " << price.cents()
                  << " (changed by itself for ALL holders!)\n"
                  << "  pointer_holder->cents()  = " << pointer_holder->cents() << " (same object)\n";
    }

}

// ============================================================
// Вариант 2: immutable Money - aliasing структурно невозможен
// ============================================================
namespace immutable_version {
    namespace {
        class Money {
        public:
            explicit Money(const long cents): cents_(cents) {}

            // "Мутирующая" операция возвращает НОВОЕ значение, а не меняет текущее.
            Money add(const long cents) { return Money{cents_ + cents}; }

            long cents() const { return cents_; }

            // C++20: автоматическое сравнение по значению всех полей -
            // не нужно вручную писать operator== и рисковать забыть поле.
            bool operator==(const Money&) const = default;

        private:
            // const - буквально невозможно изменить после конструктора
            const long cents_;
        };
    }

    static void demoNoAliasing() {
        std::cout << "\n-- immutable Money: aliasing is impossible --\n";
        Money price(1000);
        // ссылку взять можно - но у Money нет ни одного мутирующего метода
        Money& alias = price;

        // НЕ меняет price/alias - создаёт независимое новое значение
        const Money new_price = alias.add(500);
        std::cout << "  price.cents()    = " << price.cents() << " (did not change)\n";
        std::cout << "  new_price.cents() = " << new_price.cents() << " (new)\n";
        std::cout << "  price == Money(1000): " << std::boolalpha << (price == Money(1000)) << "\n";
        std::cout << std::noboolalpha;
    }

}

// ============================================================
// Потокобезопасность "бесплатно": параллельное чтение immutable Money
// не требует синхронизации в принципе - читать нечего мутировать.
// ============================================================
static void concurrentReadImmutable() {
    immutable_version::Money shared(777);
    std::vector<std::thread> threads;
    for (int i{}; i < 8; ++i) {
        threads.emplace_back([&shared]() {
            long sum{};
            for (int j{}; j < 200'000; ++j) sum += shared.cents();
            (void) sum;
        });
    }

    for (auto& t: threads) t.join();
    std::cout << "\nconcurrent read of immutable Money: finished without any mutex\n";
}

// Мутация из одного потока + чтение из другого БЕЗ синхронизации - гонка данных.
static void concurrentMutableRace() {
    mutable_version::Money shared(0);
    std::thread writer{[&shared]() {
        for (int i{}; i < 200'000; ++i) shared.add(1);
    }};

    int sum{};
    std::thread reader{[&shared, &sum]() {
        for (int i{}; i < 200'000; ++i) sum += shared.cents();
    }};

    writer.join();
    reader.join();

    std::cout << std::format("concurrent mutate+read of mutable Money: {}\n", sum);
}

int main() {
    mutable_version::demoAliasing();
    immutable_version::demoNoAliasing();
    concurrentReadImmutable();
    concurrentMutableRace();

    return 0;
}
