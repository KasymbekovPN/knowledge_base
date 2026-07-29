// Тест ТОЛЬКО высокоуровневого модуля. Обратите внимание: этот файл вообще
// не инклудит infra_stripe.hpp и не знает о Stripe. Это возможно ровно
// потому, что domain зависит от собственной абстракции IPaymentGateway,
// а не от какой-то конкретной реализации - т.е. прямое следствие DIP.

#include "domain.hpp"

#include <cassert>
#include <iostream>

class FakeGateway : public IPaymentGateway {
public:
    bool charge(const double amount) override {
        lastAmount = amount;
        return shouldSucceed;
    }

    double lastAmount{};
    bool shouldSucceed{true};
};

int main() {
    FakeGateway fake;
    OrderProcessor processor{fake};

    bool result{processor.placeOrder("widget", 42.0)};
    assert(result == true);
    assert(fake.lastAmount == 42.0);
    std::cout << "[OK] domain_test: success order\n";

    fake.shouldSucceed = false;
    result = processor.placeOrder("gadget", 10.0);
    assert(result == false);
    std::cout << "[OK] domain_test: cancelled payment\n";

    return 0;
}
