// domain.hpp — ВЫСОКОУРОВНЕВЫЙ модуль (бизнес-правила: "как оформляется заказ").
// Ключевой момент DIP: интерфейс IPaymentGateway объявлен ЗДЕСЬ, в модуле,
// которому он нужен, а не в infra-модуле, который его будет реализовывать.
// Этот заголовок НЕ включает ничего из infra_stripe - domain вообще не
// подозревает о существовании Stripe, PayPal или кого угодно ещё.

#pragma once

#include <string>

// Абстракция, которую определяет высокоуровневый модуль под свои нужды.
class IPaymentGateway {
public:
    virtual ~IPaymentGateway() = default;
    virtual bool charge(const double amount) = 0;
};

// Высокоуровневая политика: "как оформляется заказ" не зависит от того,
// КАКОЙ платёжный шлюз стоит за IPaymentGateway.
class OrderProcessor {
public:
    explicit OrderProcessor(IPaymentGateway& gateway);
    bool placeOrder(const std::string& item, const double amount) const;
private:
    IPaymentGateway& gateway_;
};
