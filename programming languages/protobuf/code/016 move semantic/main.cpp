#include <chrono>
#include <iostream>
#include <format>

#include "order.pb.h"

// элементов в одном большом Order
constexpr int K_ITEMS{200'000};

namespace {
    myapp::Order buildBigOrder(const int order_quantity) {
        myapp::Order order;
        order.set_order_id(1);
        for (int i{}; i < order_quantity; ++i) {
            myapp::Order::Item* item = order.add_items();
            item->set_sku(std::format("SKU-{}", i));
            item->set_title("Item title");
            item->set_quantity(i);
            item->set_unit_price(9.99);
        }

        // никакой ручной работы — компилятор сам выберет move/RVO
        return order;
    }

    using Clock = std::chrono::steady_clock;
    long ns_since(const Clock::time_point start) {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - start).count();
    }

    std::tuple<int> getParams(const int argc, char** argv) {
        int bufferItems{-1};
        if (argc > 1) {
            try {
                bufferItems = std::stoi(argv[1]);
            } catch (...) {}
        }

        int kItems{bufferItems >= 0 ? bufferItems : K_ITEMS};

        return {kItems};
    }

}

int main(const int argc, char *argv[]) {
    const auto [kItems] = getParams(argc, argv);

    myapp::Order original{buildBigOrder(kItems)};
    std::cout << std::format("Order with {} items is built\n", original.items_size());

    // ===== 1. Копирование (CopyFrom / copy constructor) =====
    {
        const auto t{Clock::now()};
        // вызывает copy constructor -> глубокая копия
        myapp::Order copy{original};
        const auto ns = ns_since(t);
        std::cout << std::format("Copy ctor: {} ns, items in copy: {}\n", ns, copy.items_size());
    }

    // ===== 2. Move-конструктор =====
    {
        // сначала честно копируем, чтобы не портить original
        myapp::Order source{original};
        const auto t{Clock::now()};
        // move constructor -> перенос владения буфером
        myapp::Order moved{std::move(source)};
        const auto ns{ns_since(t)};
        std::cout << std::format("Move ctor: {} ns, items in moved: {}, items in source: {}\n",
            ns,
            moved.items_size(),
            source.items_size());
    }

    // ===== 3. Swap() =====
    {
        myapp::Order a{original};
        myapp::Order b;
        b.set_order_id(999);

        const auto t{Clock::now()};
        // обмен внутренними указателями, без копирования содержимого
        a.Swap(&b);
        const auto ns{ns_since(t)};
        std::cout << std::format("Swap: {} ns, items in b: {}, items in a: {}\n", ns, b.items_size(), a.items_size());
    }

    // ===== 4. Move-присваивание (operator=(Order&&)) =====
    {
        myapp::Order source{original};
        myapp::Order target;
        const auto t{Clock::now()};
        // move assignment
        target = std::move(source);
        const auto ns{ns_since(t)};
        std::cout << std::format("Move assignment {} ns, items in target: {}\n", ns, target.items_size());
    }

    // ===== 5. Возврат по значению из функции: RVO/move, без лишней копии =====
    {
        const auto t{Clock::now()};
        // построение + возврат
        myapp::Order returned{buildBigOrder(kItems)};
        const auto ns{ns_since(t)};
        std::cout << std::format("Builder {} ns, items in returned: {}\n", ns, returned.items_size());
    }

    return 0;
}
