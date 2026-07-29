#include "domain/order_processor.hpp"
#include "infra_stripe/stripe_gateway.hpp"

int main() {
    StripeGateway gateway;
    OrderProcessor processor{gateway};
    processor.placeOrder("laptop", 999.99);

    return 0;
}
