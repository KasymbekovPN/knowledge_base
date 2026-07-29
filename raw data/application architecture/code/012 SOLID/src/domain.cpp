#include "domain.hpp"

#include <iostream>
#include <format>

OrderProcessor::OrderProcessor(IPaymentGateway& gateway):
    gateway_(gateway) {}

bool OrderProcessor::placeOrder(const std::string &item, const double amount) const {
    std::cout << std::format("[domain] placing the order {} for ${}\n", item, amount);
    const bool ok{gateway_.charge(amount)};
    if (ok) {
        std::cout << "order confirmed\n";
    } else {
        std::cout << "payment failed, order cancelled\n";
    }

    return ok;
}
