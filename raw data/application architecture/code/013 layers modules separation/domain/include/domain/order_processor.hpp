// ПУБЛИЧНЫЙ API модуля domain. Это единственное, что видят потребители
// библиотеки domain - всё остальное (domain/src/*) им недоступно физически,
// не только "по договорённости".

#pragma once

#include <string>

class IPaymentGateway {
public:
    virtual ~IPaymentGateway() = default;
    virtual bool charge(const double amount) = 0;
};

class OrderProcessor {
public:
    explicit OrderProcessor(IPaymentGateway& gateway);
    bool placeOrder(const std::string& item, const double amount);
private:
    IPaymentGateway& gateway_;
};
