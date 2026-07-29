#include "infra_stripe.hpp"

#include <iostream>
#include <format>

bool StripeGateway::charge(const double amount) {
    std::cout << std::format("StripeGateway::charge ${}\n", amount);
    return true;
}
