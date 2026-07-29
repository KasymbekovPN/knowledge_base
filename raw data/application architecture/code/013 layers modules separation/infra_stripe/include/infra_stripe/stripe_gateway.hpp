// Публичный API infra_stripe. Он ВКЛЮЧАЕТ заголовок domain, потому что
// StripeGateway реализует IPaymentGateway, объявленный в domain. Именно
// поэтому в CMake зависимость domain у infra_stripe будет PUBLIC, а не
// PRIVATE - см. комментарий в infra_stripe/CMakeLists.txt.

#pragma once

#include "domain/order_processor.hpp"

class StripeGateway: public IPaymentGateway {
public:
    bool charge(const double amount) override;
};
