#include "domain/order_processor.hpp"

#include "current_format.hpp"

#include <iostream>
#include <format>

OrderProcessor::OrderProcessor(IPaymentGateway& gateway):
    gateway_(gateway) {}

bool OrderProcessor::placeOrder(const std::string& item, const double amount) {
    std::cout << std::format("[domain] order {} for {}\n", item, formatCurrency(amount));
    bool ok{gateway_.charge(amount)};
    std::cout << std::format("{}\n", ok ? "confirmed" : "cancelled");

    return ok;
}
