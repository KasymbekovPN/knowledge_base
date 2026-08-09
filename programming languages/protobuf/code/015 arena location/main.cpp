#include <chrono>
#include <iostream>
#include <format>
#include <memory>
#include <vector>

#include <google/protobuf/arena.h>

#include "order.pb.h"

constexpr int K_ORDERS{300'000}; // сколько Order создаём за прогон
constexpr int K_ITEMS_PER_ORDER{8}; // вложенных Item на один Order

namespace {
    void fillOrder(myapp::Order* order, const int i, const int kItemsPerOrder) {
        order->set_order_id(i);
        order->set_total_amount(99.99);
        order->set_status(myapp::Order::ORDER_STATUS_PAID);
        for (int j{}; j < kItemsPerOrder; ++j) {
            myapp::Order::Item* item{order->add_items()};
            item->set_sku(std::format("SKU-{}", j));
            item->set_title("Item title");
            item->set_quantity(j + 1);
            item->set_unit_price(9.99);
        }
    }

    using Clock = std::chrono::steady_clock;
    long ms_since(Clock::time_point start) {
        return std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - start).count();
    }

    void logDuration(const std::string& label, long create_ms, const long destroy_ms) {
        std::cout << std::format("{}: creation: {} ms, destroying: {} ms, total {} ms\n",
            label,
            create_ms,
            destroy_ms,
            create_ms + destroy_ms);
    }

    std::tuple<int, int> getParams(const int argc, char** argv) {
        int bufferOrders{-1};
        if (argc > 1) {
            try {
                bufferOrders = std::stoi(argv[1]);
            } catch (...) {}
        }

        int bufferItemsPerOrder{-1};
        if (argc > 2) {
            try {
                bufferItemsPerOrder = std::stoi(argv[2]);
            } catch (...) {}
        }

        int kOrders{bufferOrders >= 0 ? bufferOrders : K_ORDERS};
        int kItemsPerOrder{bufferItemsPerOrder >= 0 ? bufferItemsPerOrder : K_ITEMS_PER_ORDER};

        return {kOrders, kItemsPerOrder};
    }

}

int main(const int argc, char *argv[]) {
    const auto [kOrders, kItemsPerOrder] = getParams(argc, argv);

    std::cout << std::format("{} Order x {} Item = {} protobuf-objects\n",
        kOrders,
        kItemsPerOrder,
        static_cast<long long>(kOrders) * (kItemsPerOrder + 1));

    // ===== 1. Куча: раздельно меряем создание и уничтожение =====
    {
        std::vector<myapp::Order*> orders;
        orders.reserve(kOrders);

        const auto t_create = Clock::now();
        for (int i{}; i < kOrders; ++i) {
            myapp::Order* order{new myapp::Order()};
            fillOrder(order, i, kItemsPerOrder);
            orders.push_back(order);
        }
        const long create_ms{ms_since(t_create)};

        const auto t_destroy{Clock::now()};
        for (const auto* order : orders) delete order; // рекурсивный обход дерева + N+M free()
        const long destroy_ms{ms_since(t_destroy)};

        logDuration("Heap", create_ms, destroy_ms);
    }

    // ===== 2. Arena: раздельно меряем создание и уничтожение =====
    {
        auto arena{std::make_unique<google::protobuf::Arena>()};
        std::vector<myapp::Order*> orders;
        orders.reserve(kOrders);

        const auto t_create = Clock::now();
        for (int i{}; i < kOrders; ++i) {
            myapp::Order* order{
                google::protobuf::Arena::Create<myapp::Order>(arena.get())
            };
            fillOrder(order, i, kItemsPerOrder);
            orders.push_back(order);
        }
        const long create_ms{ms_since(t_create)};

        const auto t_destroy{Clock::now()};
        arena.reset(); // одно освобождение памяти всей арены целиком
        const long destroy_ms{ms_since(t_destroy)};

        logDuration("Arena", create_ms, destroy_ms);
    }

    return 0;
}
