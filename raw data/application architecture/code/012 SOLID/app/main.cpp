// Composition root: единственное место во всей программе, где domain
// и infra встречаются. Здесь оба #include легальны и ожидаемы.

#include "domain.hpp"
#include "infra_stripe.hpp"

int main() {
    // low-level деталь
    StripeGateway gateway;
    // high-level политика, DI через конструктор
    OrderProcessor processor{gateway};
    processor.placeOrder("laptop", 999.99);

    return 0;
}
