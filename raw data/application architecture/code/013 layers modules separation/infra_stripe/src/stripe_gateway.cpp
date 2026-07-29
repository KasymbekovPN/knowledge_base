#include "infra_stripe/stripe_gateway.hpp"

#include <iostream>
#include <format>

bool StripeGateway::charge(const double amount) {
    std::cout << std::format("[Stripe SDK] StripeGateway::charge ${}\n", amount);
    return true;
}
